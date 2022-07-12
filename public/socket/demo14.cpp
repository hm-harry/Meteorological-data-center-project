/*
 * 程序名：demo14.cpp，此程序用于演示网银APP软件的服务端，增加了心跳报文。
 * 作者：吴从周
*/
#include "../_public.h"

CLogFile logfile;      // 服务程序的运行日志。
CTcpServer TcpServer;  // 创建服务端对象。

void FathEXIT(int sig);  // 父进程退出函数。
void ChldEXIT(int sig);  // 子进程退出函数。

bool bsession=false;     // 客户端是否已登录：true-已登录;false-未登录或登录失败。

// 处理业务的主函数。
bool _main(const char *strrecvbuffer,char *strsendbuffer);

// 心跳。
bool srv000(const char *strrecvbuffer,char *strsendbuffer);

// 登录业务处理函数。
bool srv001(const char *strrecvbuffer,char *strsendbuffer);

// 查询余额业务处理函数。
bool srv002(const char *strrecvbuffer,char *strsendbuffer);

// 转账。
bool srv003(const char *strrecvbuffer,char *strsendbuffer);
 
int main(int argc,char *argv[])
{
  if (argc!=4)
  {
    printf("Using:./demo14 port logfile timeout\nExample:./demo14 5005 /tmp/demo14.log 35\n\n"); return -1;
  }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,FathEXIT); signal(SIGTERM,FathEXIT);

  if (logfile.Open(argv[2],"a+")==false) { printf("logfile.Open(%s) failed.\n",argv[2]); return -1; }

  // 服务端初始化。
  if (TcpServer.InitServer(atoi(argv[1]))==false)
  {
    logfile.Write("TcpServer.InitServer(%s) failed.\n",argv[1]); return -1;
  }

  while (true)
  {
    // 等待客户端的连接请求。
    if (TcpServer.Accept()==false)
    {
      logfile.Write("TcpServer.Accept() failed.\n"); FathEXIT(-1);
    }

    logfile.Write("客户端（%s）已连接。\n",TcpServer.GetIP());

    if (fork()>0) { TcpServer.CloseClient(); continue; }  // 父进程继续回到Accept()。
   
    // 子进程重新设置退出信号。
    signal(SIGINT,ChldEXIT); signal(SIGTERM,ChldEXIT);

    TcpServer.CloseListen();

    // 子进程与客户端进行通讯，处理业务。
    char strrecvbuffer[1024],strsendbuffer[1024];

    // 与客户端通讯，接收客户端发过来的报文后，回复ok。
    while (1)
    {
      memset(strrecvbuffer,0,sizeof(strrecvbuffer));
      memset(strsendbuffer,0,sizeof(strsendbuffer));

      if (TcpServer.Read(strrecvbuffer,atoi(argv[3]))==false) break; // 接收客户端的请求报文。
      logfile.Write("接收：%s\n",strrecvbuffer);

      // 处理业务的主函数。
      if (_main(strrecvbuffer,strsendbuffer)==false) break;

      if (TcpServer.Write(strsendbuffer)==false) break; // 向客户端发送响应结果。
      logfile.Write("发送：%s\n",strsendbuffer);
    }

    ChldEXIT(0);
  }
}

// 父进程退出函数。
void FathEXIT(int sig)  
{
  // 以下代码是为了防止信号处理函数在执行的过程中被信号中断。
  signal(SIGINT,SIG_IGN); signal(SIGTERM,SIG_IGN);

  logfile.Write("父进程退出，sig=%d。\n",sig);

  TcpServer.CloseListen();    // 关闭监听的socket。

  kill(0,15);     // 通知全部的子进程退出。

  exit(0);
}

// 子进程退出函数。
void ChldEXIT(int sig)  
{
  // 以下代码是为了防止信号处理函数在执行的过程中被信号中断。
  signal(SIGINT,SIG_IGN); signal(SIGTERM,SIG_IGN);

  logfile.Write("子进程退出，sig=%d。\n",sig);

  TcpServer.CloseClient();    // 关闭客户端的socket。

  exit(0);
}

// 处理业务的主函数。
bool _main(const char *strrecvbuffer,char *strsendbuffer)
{
  // 解析strrecvbuffer，获取服务代码（业务代码）。
  int isrvcode=-1;
  GetXMLBuffer(strrecvbuffer,"srvcode",&isrvcode);

  if ( (isrvcode!=1) && (bsession==false) )
  {
    strcpy(strsendbuffer,"<retcode>-1</retcode><message>用户未登录。</message>"); return true;
  }

  // 处理每种业务。
  switch (isrvcode)
  {
    case 0:   // 心跳。
      srv000(strrecvbuffer,strsendbuffer); break;
    case 1:   // 登录。
      srv001(strrecvbuffer,strsendbuffer); break;
    case 2:   // 查询余额。
      srv002(strrecvbuffer,strsendbuffer); break;
    case 3:   // 转账。
      srv003(strrecvbuffer,strsendbuffer); break;
    default:
      logfile.Write("业务代码不合法：%s\n",strrecvbuffer); return false;
  }

  return true;
}

// 心跳。
bool srv000(const char *strrecvbuffer,char *strsendbuffer)
{
  strcpy(strsendbuffer,"<retcode>0</retcode><message>成功。</message>");
  
  return true;
}

// 登录。
bool srv001(const char *strrecvbuffer,char *strsendbuffer)
{
  // <srvcode>1</srvcode><tel>1392220000</tel><password>123456</password>

  // 解析strrecvbuffer，获取业务参数。
  char tel[21],password[31];
  GetXMLBuffer(strrecvbuffer,"tel",tel,20);
  GetXMLBuffer(strrecvbuffer,"password",password,30);

  // 处理业务。
  // 把处理结果生成strsendbuffer。
  if ( (strcmp(tel,"1392220000")==0) && (strcmp(password,"123456")==0) )
  {
    strcpy(strsendbuffer,"<retcode>0</retcode><message>成功。</message>");  bsession=true;
  }
  else
    strcpy(strsendbuffer,"<retcode>-1</retcode><message>失败。</message>");

  return true;
}

// 查询余额业务处理函数。
bool srv002(const char *strrecvbuffer,char *strsendbuffer)
{
  // <srvcode>2</srvcode><cardid>62620000000001</cardid>

  // 解析strrecvbuffer，获取业务参数。
  char cardid[31];
  GetXMLBuffer(strrecvbuffer,"cardid",cardid,30);

  // 处理业务。
  // 把处理结果生成strsendbuffer。
  if (strcmp(cardid,"62620000000001")==0) 
    strcpy(strsendbuffer,"<retcode>0</retcode><message>成功。</message><ye>100.58</ye>");
  else
    strcpy(strsendbuffer,"<retcode>-1</retcode><message>失败。</message>");

  return true;
}

// 转账。
bool srv003(const char *strrecvbuffer,char *strsendbuffer)
{
  // 编写转账业务的代码。

  strcpy(strsendbuffer,"<retcode>0</retcode><message>成功。</message><ye>100.58</ye>");

  return true;
}

