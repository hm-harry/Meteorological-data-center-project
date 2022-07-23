/*
 * 程序名：tcpgetfiles.cpp，采用tcp协议，实现文件下载的客户端
 * 作者：吴惠明。
*/
#include "_public.h"

CTcpClient TcpClient;
CLogFile logfile;
CPActive PActive; // 进程心跳

// 程序运行的参数结构体
struct st_arg{
  int clienttype; // 客户端类型，1-上传文件；2-下载文件。
  char ip[31]; // 客户端的IP地址
  int port; // 客户端的端口
  int ptype; // 文件下载成功后服务端文件的处理方式1-删除文件，2-移动到备份目录
  char srvpath[301]; // 服务端文件存放的根目录
  char srvpathbak[301]; // 文件成功下载后，服务端文件备份的根目录，当ptype==2时有效
  bool andchild; // 是否下载srvpath目录下各级子目录的文件，true-是；false-否
  char matchname[301]; // 待下载文件名的匹配方式，如"*.txt,*.XML",注意大写
  char clientpath[301]; // 客户端文件存放的根目录
  int timetvl; // 扫描服务端目录的时间间隔，单位秒
  int timeout; // 进程心跳的超时时间
  char pname[51]; // 进程名，用"tcpgetfiles_后缀"的方式
}starg;

char strrecvbuffer[1024]; // 发送报文的buffer
char strsendbuffer[1024]; // 接收报文的buffer

// 程序退出和信号2.5的处理函数
void EXIT(int sig);

void _help();

// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer);

bool Login(const char* argv); // 登录业务

// 文件下载主函数
void _tcpgetfiles();

// 接收文件的内容
bool RecvFile(const int sockfd, const char* filename, const char* mtime, int filesize);

int main(int argc,char *argv[])
{
  if (argc!=3){
    _help(); 
    return -1;
  }

  CloseIOAndSignal(); signal(SIGINT, EXIT); signal(SIGTERM, EXIT);

  // 打开日志文件
  if(logfile.Open(argv[1], "a+") == false) return -1;

  // 解析xml，得到程序运行的参数
  if(_xmltoarg(argv[2]) == false) return -1;

  PActive.AddPInfo(starg.timeout, starg.pname); // 把进程的心跳信息写入共享内存。

  if(TcpClient.ConnectToServer(starg.ip, starg.port) == false){
    logfile.Write("TcpClient.ConnectToServer(%s, %d) failed\n", starg.ip, starg.port);
    EXIT(-1);
  }else{
    logfile.Write("TcpClient.ConnectToServer(%s, %d) successfully\n", starg.ip, starg.port);
  }

  // 登录业务
  if(Login(argv[2]) == false){
    logfile.Write("Login() failed.\n");
    EXIT(-1);
  }
  
  // 调用文件下载的主函数
  _tcpgetfiles();
}

// 登录业务
bool Login(const char* argv){
  memset(strsendbuffer, 0, sizeof(strsendbuffer));
  memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

  SPRINTF(strsendbuffer, sizeof(strsendbuffer), "%s<clienttype>2</clienttype>", argv);
  logfile.Write("发送：%s\n", strsendbuffer);
  if(TcpClient.Write(strsendbuffer) == false) return false;

  if(TcpClient.Read(strrecvbuffer, 20) == false) return false;
  logfile.Write("接收：%s\n", strrecvbuffer);

  logfile.Write("登录(%s, %d)成功。\n", starg.ip, starg.port);
  return true;
}

void EXIT(int sig){
  logfile.Write("程序退出，sig = %d\n\n", sig);
  exit(0);
}

void _help(){
  printf("\n");
  printf("Using:/project/tools1/bin/tcpgetfiles logfilename xmlbuffer\n\n");

  printf("Sample:/project/tools1/bin/procctl 20 /project/tools1/bin/tcpgetfiles /log/idc/tcpgetfiles_surfdata.log \"<ip>172.29.193.250</ip><port>5005</port><ptype>1</ptype><srvpath>/tmp/tcp/surfdata2</srvpath><srvpathbak>/tmp/tcp/surfdata2bak</srvpathbak><andchild>true</andchild><matchname>*.XML,*.CSV,*.JSON</matchname><clientpath>/tmp/tcp/surfdata3</clientpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpgetfiles_surfdata</pname>\"\n\n\n");

  printf("本程序是数据中心的公共功能模块，采用tcp协议从服务器下载文件。\n");
  printf("logfilename      本程序运行的日志文件\n");
  printf("xmlbuffer        本程序运行的参数，如下：\n");
  printf("ip               客户端的IP地址\n");
  printf("port             客户端的端口\n");
  printf("ptype            文件下载成功后文件的处理方式1-删除文件，2-移动到备份目录\n");
  printf("srvpath          服务端文件存放的根目录\n");
  printf("srvpathbak       服务端文件存备份的根目录\n");
  printf("andchild         是否下载clientpath目录下各级子目录的文件，true-是；false-否\n");
  printf("matchname        待下载文件名的匹配方式，如\"*.txt,*.XML\",注意大写\n");
  printf("clientpath       本地文件存放的根目录\n");
  printf("timetvl          扫描本地目录的时间间隔，单位秒\n");
  printf("timeout          进程心跳的超时时间\n");
  printf("pname            进程名，尽可能采用易懂的、与其他进程不同的名称方便故障排查。\n");
}

bool _xmltoarg(char* strxmlbuffer){
  memset(&starg, 0, sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuffer, "ip", starg.ip);
  if(strlen(starg.ip) == 0){logfile.Write("ip is null.\n"); return false;}

  GetXMLBuffer(strxmlbuffer, "port", &starg.port);
  if(starg.port == 0){logfile.Write("port is null.\n"); return false;}

  GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype);

  GetXMLBuffer(strxmlbuffer, "srvpath", starg.srvpath);

  GetXMLBuffer(strxmlbuffer, "srvpathbak", starg.srvpathbak);

  GetXMLBuffer(strxmlbuffer, "andchild", &starg.andchild);

  GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname);

  GetXMLBuffer(strxmlbuffer, "clientpath", starg.clientpath);

  GetXMLBuffer(strxmlbuffer, "timetvl", &starg.timetvl);
  if(starg.timetvl > 30) starg.timetvl = 30; // 扫描本地文件的时间间隔，没有必要超过30秒

  GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout);
  if(starg.timeout < 50) starg.timeout = 50;// 进程心跳的超时时间，没有必要小于50秒

  GetXMLBuffer(strxmlbuffer, "pname", starg.pname);

  return true;
}

// 文件下载主函数
void _tcpgetfiles(){
  PActive.AddPInfo(starg.timeout, starg.pname);

  while(true){
    memset(strsendbuffer, 0, sizeof(strsendbuffer));
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

    PActive.UptATime(); // 把进程的心跳信息进行更新
    
    // 接收服务端的报文
    if(TcpClient.Read(strrecvbuffer, starg.timetvl + 10) == false){
      logfile.Write("TcpClient.Read() failed.\n");
      return;
    }
    // logfile.Write("strrecvbuffer = %s\n", strrecvbuffer);

    // 处理心跳报文
    if(strcmp(strrecvbuffer, "<activetest>ok</activetest>") == 0){
      strcpy(strsendbuffer, "ok");
      // logfile.Write("strsendbuffer = %s\n", strsendbuffer);
      if(TcpClient.Write(strsendbuffer) == false){
        logfile.Write("TcpClient.Write() failed.\n");
        return;
      }
    }

    // 处理下载文件的请求报文
    if(strncmp(strrecvbuffer, "<filename>", 10) == 0){
      // 解析上传文件请求报文的xml
      char serverfilename[301]; memset(serverfilename, 0, sizeof(serverfilename));
      char mtime[21]; memset(mtime, 0, sizeof(mtime));
      int filesize = 0;
      GetXMLBuffer(strrecvbuffer, "filename", serverfilename, 300);
      GetXMLBuffer(strrecvbuffer, "mtime", mtime, 19);
      GetXMLBuffer(strrecvbuffer, "size", &filesize);

      // 客户端和服务端文件的目录是不一样的，以下代码生成客户器的文件名
      // 把文件名中的srvpath替换成clientpath，要小心第三个参数
      char clientfilename[301]; memset(clientfilename, 0, sizeof(clientfilename));
      strcpy(clientfilename, serverfilename);
      UpdateStr(clientfilename, starg.srvpath, starg.clientpath, false);

      // 接收文件的内容
      logfile.Write("recv %s(%d) ...", clientfilename, filesize);
      if(RecvFile(TcpClient.m_connfd, clientfilename, mtime, filesize) == true){
        logfile.WriteEx("ok.\n");
        SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><result>ok</result>", serverfilename);
      }else{
        logfile.WriteEx("failed.\n");
        SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><result>failed</result>", serverfilename);
      }

      // 把接收结果返回给对端
      // logfile.Write("strsendbuffer = %s\n", strsendbuffer);
      if(TcpClient.Write(strsendbuffer) == false){
        logfile.Write("TcpClient.Write() failed.\n");
        return;
      }

    }
  }
}

// 接收文件的内容
bool RecvFile(const int sockfd, const char* filename, const char* mtime, int filesize){
  // 生成临时文件名
  char strfilenametmp[301];
  SNPRINTF(strfilenametmp, sizeof(strfilenametmp), 300, "%s.tmp", filename);

  int totalbytes = 0; // 已接收文件的总字节数
  int onread = 0; // 本次打算接收的字节数
  char buffer[1000]; // 接收文件内容的缓冲区
  FILE* fp = NULL;

  // 创建临时文件
  if((fp = FOPEN(strfilenametmp, "wb")) == NULL){
    return false;
  }

  while(true){
    memset(buffer, 0, sizeof(buffer));
    // 计算本次应该接收的字节数
    if(filesize - totalbytes > 1000) onread = 1000;
    else onread = filesize - totalbytes;

    // 接收文件内容
    if(Readn(sockfd, buffer, onread) == false){
      fclose(fp);
      return false;
    }

    // 把接收到的内容写入文件
    fwrite(buffer, 1, onread, fp);

    // 计算已接收到的总字节数，如果文件接收完毕，跳出循环
    totalbytes = totalbytes + onread;

    if(totalbytes == filesize) break;
  }

  // 关闭临时文件
  fclose(fp);

  // 重置临时文件的时间
  UTime(strfilenametmp, mtime);

  // 把临时文件RENAME为正式文件
  if(RENAME(strfilenametmp, filename) == false) return false;

  return true;
}