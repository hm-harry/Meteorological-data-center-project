/*
 * 程序名：demo10.cpp，此程序演示采用开发框架的CTcpServer类实现socket通信多进程的服务器。
 1.在多进程的服务程序中，如果杀掉一个子进程，和这个子进程通讯的客户端会断开，但是，不会影响其他的进程和客户端，也不会影响父进程
 2.如果杀掉父进程，不会影响正在通讯的子进程，但是，新的客户端无法建立连接
 3.如果用killall+程序名，可以杀掉父进程和所有的子进程

 多进程网络服务端程序退出的三种情况：
1.如果是子进程收到退出信号，该子进程断开与客户端连接的socket，然后退出。
2.如果是父进程收到退出信号，父进程先关闭监听的socket，然后向全部的子进程发出退出信号。
3.如果父子进程都收到了退出信号，本质上与第二种情况相同。
 * 作者：吴惠明
*/
#include "../_public.h"

CLogFile logfile; // 服务程序的运行日志
CTcpServer TcpServer; // 创建服务端对象

void FathEXIT(int sig); // 父进程退出函数
void ChldEXIT(int sig); // 子进程退出函数

int main(int argc,char *argv[])
{
  if (argc!=3)
  {
    printf("Using:./demo10 port logfile\nExample:./demo10 5005 /tmp/demo10.log\n\n"); return -1;
  }

  CloseIOAndSignal(); signal(SIGINT, FathEXIT); signal(SIGTERM, FathEXIT);

  if(logfile.Open(argv[2], "a+") == false){
    printf("logfile.Open(%s) failed\n", argv[2]);
    return -1;
  }

  // 服务端初始化
  if(TcpServer.InitServer(atoi(argv[1])) == false){
    logfile.Write("TcpServer.InitServer(%s) failed.\n", argv[1]);
    return -1;
  }
  while(true){
    // 等待客户端的连接
    if(TcpServer.Accept() == false){
      logfile.Write("TcpServer.Accept() failed.\n");
      FathEXIT(-1);
      // return -1;
    }

    logfile.Write("客户端（%s）已连接。\n", TcpServer.GetIP());
// printf("listenfd = %d, connfd = %d\n", TcpServer.m_listenfd, TcpServer.m_connfd);

    if(fork() > 0) { // 父进程继续回到Accept();
      TcpServer.CloseClient();
      continue;
    }
    // 子进程重新设置退出信号。
    signal(SIGINT, ChldEXIT); signal(SIGTERM, ChldEXIT);

    TcpServer.CloseListen();
    // 子进程与客户端进行通讯，处理业务
    char buffer[102400];

    // 与客户端通讯，接收客户端发过来的报文后，回复ok。
    while (1)
    {
      memset(buffer,0,sizeof(buffer));
      if ( (TcpServer.Read(buffer)) == false) break;// 接收客户端的请求报文。
      
      logfile.Write("接收：%s\n",buffer);

      strcpy(buffer,"ok");
      if ( (TcpServer.Write(buffer)) == false) break; // 向客户端发送响应结果。

      logfile.Write("发送：%s\n",buffer);
    }

    ChldEXIT(0);
    // return 0;
  }
}

void FathEXIT(int sig){// 父进程退出函数
// 以下代码是为了防止信号处理函数在执行的过程中被信号中断
  signal(SIGINT, SIG_IGN); signal(SIGTERM, SIG_IGN);
  logfile.Write("父进程退出。sig = %d.\n", sig);
  TcpServer.CloseListen(); // 关闭监听的socket
  kill(0, 15); // 通知全部的子进程退出
  exit(0);
}

void ChldEXIT(int sig){// 子进程退出函数
// 以下代码是为了防止信号处理函数在执行的过程中被信号中断
  signal(SIGINT, SIG_IGN); signal(SIGTERM, SIG_IGN);
  logfile.Write("子进程退出。sig = %d.\n", sig);
  TcpServer.CloseClient(); // 关闭客户端的socket
  exit(0);
}