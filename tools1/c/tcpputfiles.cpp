/*
 * 程序名：tcpputfiles.cpp，采用tcp协议，实现文件上传的客户端
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
  int ptype; // 文件上传成功后文件的处理方式1-删除文件，2-移动到备份目录
  char clientpath[301]; // 本地文件存放的根目录
  char clientpathbak[301]; // 文件成功上传后，本地文件备份的根目录，当ptype==2时有效
  bool andchild; // 是否上传clientpath目录下各级子目录的文件，true-是；false-否
  char matchname[301]; // 待上传文件名的匹配方式，如"*.txt,*.XML",注意大写
  char srvpath[301]; // 服务端文件存放的根目录
  int timetvl; // 扫描本地目录的时间间隔，单位秒
  int timeout; // 进程心跳的超时时间
  char pname[51]; // 进程名，用"tcpgetfiles_后缀"的方式
}starg;

char strrecvbuffer[1024]; // 发送报文的buffer
char strsendbuffer[1024]; // 接收报文的buffer

bool bcontinue = true; // 如果调用_tcpputfiles发送了文件，bcontinue为true，初始化为true

// 程序退出和信号2.5的处理函数
void EXIT(int sig);

void _help();

// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer);

bool ActiveTest(); // 心跳

bool Login(const char* argv); // 登录业务

// 文件上传主函数，执行一次文件上传任务
bool _tcpputfiles();

// 把文件的内容发送给对端
bool SendFile(const int sockfd, const char* filename, const int filesize);

// 删除或者转存本地的文件
bool AckMessage(const char* strrecvbuffer);

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
  
  while(true){
    // 调用文件上传的主函数，执行一次文件上传的任务
    if(_tcpputfiles() == false){
      logfile.Write("_tcpputfiles() failed.\n");
      EXIT(-1);
    }

    if(bcontinue == false){
      sleep(starg.timetvl);
      // 心跳
      if(ActiveTest() == false) break;
    }
    PActive.UptATime(); // 把进程的心跳信息进行更新
  }

  EXIT(0);
}

// 心跳
bool ActiveTest(){
  memset(strsendbuffer, 0, sizeof(strsendbuffer));
  memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

  SPRINTF(strsendbuffer, sizeof(strsendbuffer), "<activetest>ok</activetest>");
  // logfile.Write("发送：%s\n", strsendbuffer);
  if(TcpClient.Write(strsendbuffer) == false) return false;

  if(TcpClient.Read(strrecvbuffer, 20) == false) return false;
  // logfile.Write("接收：%s\n", strrecvbuffer);

  return true;
}

// 登录业务
bool Login(const char* argv){
  memset(strsendbuffer, 0, sizeof(strsendbuffer));
  memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

  SPRINTF(strsendbuffer, sizeof(strsendbuffer), "%s<clienttype>1</clienttype>", argv);
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
  printf("Using:/project/tools1/bin/tcpputfiles logfilename xmlbuffer\n\n");

  printf("Sample:/project/tools1/bin/procctl 20 /project/tools1/bin/tcpputfiles /tmp/tcpputfiles_surfdata.log \"<ip>172.29.193.250</ip><port>5005</port><ptype>1</ptype><clientpath>/tmp/tcp/surfdata1</clientpath><clientpathbak>/tmp/tcp/surfdata1bak</clientpathbak><andchild>true</andchild><matchname>*.XML,*.CSV</matchname><srvpath>/tmp/tcp/surfdata2</srvpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_surfdata</pname>\"\n\n\n");

  printf("本程序是数据中心的公共功能模块，采用tcp协议把文件发送给服务器。\n");
  printf("logfilename      本程序运行的日志文件\n");
  printf("xmlbuffer        本程序运行的参数，如下：\n");
  printf("ip               客户端的IP地址\n");
  printf("port             客户端的端口\n");
  printf("ptype            文件上传成功后文件的处理方式1-删除文件，2-移动到备份目录\n");
  printf("clientpath       本地文件存放的根目录\n");
  printf("clientpathbak    文件成功上传后，本地文件备份的根目录，当ptype==2时有效\n");
  printf("andchild         是否上传clientpath目录下各级子目录的文件，true-是；false-否\n");
  printf("matchname        待上传文件名的匹配方式，如\"*.txt,*.XML\",注意大写\n");
  printf("srvpath          服务端文件存放的根目录\n");
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
  if(starg.ptype == 0){logfile.Write("ptype is null.\n"); return false;}

  GetXMLBuffer(strxmlbuffer, "clientpath", starg.clientpath);
  if(strlen(starg.clientpath) == 0){logfile.Write("clientpath is null.\n"); return false;}

  GetXMLBuffer(strxmlbuffer, "clientpathbak", starg.clientpathbak);
  if(strlen(starg.clientpathbak) == 0){logfile.Write("clientpathbak is null.\n"); return false;}

  GetXMLBuffer(strxmlbuffer, "andchild", &starg.andchild);

  GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname);
  if(strlen(starg.matchname) == 0){logfile.Write("matchname is null.\n"); return false;}

  GetXMLBuffer(strxmlbuffer, "srvpath", starg.srvpath);
  if(strlen(starg.srvpath) == 0){logfile.Write("srvpath is null.\n"); return false;}

  GetXMLBuffer(strxmlbuffer, "timetvl", &starg.timetvl);
  if(starg.timetvl == 0){logfile.Write("timetvl is null.\n"); return false;}
  if(starg.timetvl > 30) starg.timetvl = 30; // 扫描本地文件的时间间隔，没有必要超过30秒

  GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout);
  if(starg.timeout == 0){logfile.Write("timeout is null.\n"); return false;}
  if(starg.timeout < 50) starg.timeout = 50;// 进程心跳的超时时间，没有必要小于50秒

  GetXMLBuffer(strxmlbuffer, "pname", starg.pname);
  if(strlen(starg.pname) == 0){logfile.Write("pname is null.\n"); return false;}

  return true;
}

// 文件上传主函数，执行一次文件上传任务
bool _tcpputfiles(){
  CDir Dir;
  // 调用OpenDir()打开starg.clientpath目录
  if(Dir.OpenDir(starg.clientpath, starg.matchname, 10000, starg.andchild) == false){
    logfile.Write("Dir.OpenDir(%s) failed.\n", starg.clientpath);
  }

  int buflen = 0; // 用于存放strrecvbuffer的长度
  int delayed = 0; // 未收到对端确认报文的文件数量

  bcontinue = false;

  while(true){
    memset(strsendbuffer, 0, sizeof(strsendbuffer));
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

    // 遍历目录中的每个文件，调用ReadDir(获取文件名)
    if(Dir.ReadDir() == false) break;

    bcontinue = true;

    // 把文件名、修改时间、文件大小组成报文，发送给对端
    SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><mtime>%s</mtime><size>%d</size>", Dir.m_FullFileName, Dir.m_ModifyTime, Dir.m_FileSize);

    // logfile.Write("strsendbuffer = %s\n", strsendbuffer);
    if(TcpClient.Write(strsendbuffer) == false){
      logfile.Write("strsendbuffer.Write() failed\n");
      return false;
    }

    // 把文件的内容发送给对端
    logfile.Write("send %s(%d) ...", Dir.m_FullFileName, Dir.m_FileSize);
    if(SendFile(TcpClient.m_connfd, Dir.m_FullFileName, Dir.m_FileSize) == true){
      logfile.WriteEx("ok.\n"); delayed++;
    }else{
      logfile.WriteEx("failed.\n");
      TcpClient.Close();
      return false;
    }

    PActive.UptATime(); // 把进程的心跳信息进行更新

    // 接收对端的确认报文
    while(delayed > 0){
      memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
      if(TcpRead(TcpClient.m_connfd, strrecvbuffer, &buflen, -1) == false) break;
      // 删除或者转存本地的文件
      AckMessage(strrecvbuffer);
      delayed--;
    }
  }
  // 继续接收对端的确认报文
  while(delayed > 0){
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
    if(TcpRead(TcpClient.m_connfd, strrecvbuffer, &buflen, 10) == false) break;
    // 删除或者转存本地的文件
    AckMessage(strrecvbuffer);
    delayed--;
  }
  return true;
}

// 把文件的内容发送给对端
bool SendFile(const int sockfd, const char* filename, const int filesize){
  int onread = 0; // 每次调用fread时打算读取的字节数
  int bytes = 0; // 调用一次fread从文件中读取的字节数
  char buffer[1000]; // 存放读取数据的buffer
  int totalbytes = 0; // 从文件中已读取的字节总数
  FILE *fd = NULL;

  // 以"rb"的模式打开文件。
  if((fd = fopen(filename, "rb")) == NULL){
    return false;
  }

  while(true){
    memset(buffer, 0, sizeof(buffer));
    // 计算本次应该读取的字节数，如果剩余的数据超过1000字节，就打算读1000字节
    if(filesize - totalbytes > 1000) onread = 1000;
    else onread = filesize - totalbytes;

    // 从文件中读取数据
    bytes = fread(buffer, 1, onread, fd);

    // 把读取到的数据发给对端
    if(bytes > 0){
      if(Writen(sockfd, buffer, bytes) == false){
        fclose(fd);
        return false;
      }
    }

    // 计算文件已读取的字节总数，如果文件已读完，跳出循环
    totalbytes = totalbytes + bytes;
    if(totalbytes == filesize) break;
  }
  fclose(fd);
  return true;
}

// 删除或者转存本地的文件
bool AckMessage(const char* strrecvbuffer){
  char filename[301];
  char result[11];

  memset(filename, 0, sizeof(filename));
  memset(result, 0, sizeof(result));

  GetXMLBuffer(strrecvbuffer, "filename", filename, 300);
  GetXMLBuffer(strrecvbuffer, "result", result, 10);

  // 如果服务端接收文件不成功，直接返回
  if(strcmp(result, "ok") != 0) return true;

  // ptype == 1删除文件
  if(starg.ptype == 1){
    if(REMOVE(filename) == false){
      logfile.Write("REMOVE(%s) failed.\n", filename);
      return false;
    }
  }

  // ptype == 2移动到备份目录
  if(starg.ptype == 2){
    char bakfilename[301];
    STRCPY(bakfilename, sizeof(bakfilename), filename);
    UpdateStr(bakfilename, starg.clientpath, starg.clientpathbak, false);
    if(RENAME(filename, bakfilename) == false){
      logfile.Write("RENAME(%s,%s) failed.\n", filename, bakfilename);
      return false;
    }
  }
  return true;
}
