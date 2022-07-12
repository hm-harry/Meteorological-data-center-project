/*
 * 程序名：demo20.cpp，此程序演示采用开发框架的CTcpServer类实现socket通讯多线程的服务端。
 * 作者：吴从周
*/
#include "../_public.h"

CLogFile   logfile;    // 服务程序的运行日志。
CTcpServer TcpServer;  // 创建服务端对象。

void EXIT(int sig);    // 进程的退出函数。

pthread_spinlock_t vthidlock;  // 用于锁定vthid的自旋锁。
vector<pthread_t> vthid;       // 存放全部线程id的容器。
void *thmain(void *arg);       // 线程主函数。

void thcleanup(void *arg);     // 线程清理函数。
 
int main(int argc,char *argv[])
{
  if (argc!=3)
  {
    printf("Using:./demo20 port logfile\nExample:./demo20 5005 /tmp/demo20.log\n\n"); return -1;
  }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[2],"a+")==false) { printf("logfile.Open(%s) failed.\n",argv[2]); return -1; }

  // 服务端初始化。
  if (TcpServer.InitServer(atoi(argv[1]))==false)
  {
    logfile.Write("TcpServer.InitServer(%s) failed.\n",argv[1]); return -1;
  }

  pthread_spin_init(&vthidlock,0);

  while (true)
  {
    // 等待客户端的连接请求。
    if (TcpServer.Accept()==false)
    {
      logfile.Write("TcpServer.Accept() failed.\n"); EXIT(-1);
    }

    logfile.Write("客户端（%s）已连接。\n",TcpServer.GetIP());

    // 创建一个新的线程，让它与客户端通讯。
    pthread_t thid;
    if (pthread_create(&thid,NULL,thmain,(void *)(long)TcpServer.m_connfd)!=0)
    {
      logfile.Write("pthread_create() failed.\n"); TcpServer.CloseListen(); continue;
    }

    pthread_spin_lock(&vthidlock);
    vthid.push_back(thid);    // 把线程id放入容器。
    pthread_spin_unlock(&vthidlock);
  }
}

void *thmain(void *arg)     // 线程主函数。
{
  pthread_cleanup_push(thcleanup,arg);       // 把线程清理函数入栈（关闭客户端的socket）。

  int connfd=(int)(long)arg;    // 客户端的socket。

  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);   // 线程取消方式为立即取消。

  pthread_detach(pthread_self());           // 把线程分离出去。

  // 子线程与客户端进行通讯，处理业务。
  int  ibuflen;
  char buffer[102400];

  // 与客户端通讯，接收客户端发过来的报文后，回复ok。
  while (1)
  {
    memset(buffer,0,sizeof(buffer));
    if (TcpRead(connfd,buffer,&ibuflen,30)==false) break; // 接收客户端的请求报文。
    logfile.Write("接收：%s\n",buffer);

    strcpy(buffer,"ok");
    if (TcpWrite(connfd,buffer)==false) break; // 向客户端发送响应结果。
    logfile.Write("发送：%s\n",buffer);
  }

  close(connfd);       // 关闭客户端的连接。

  // 把本线程id从存放线程id的容器中删除。
  pthread_spin_lock(&vthidlock);
  for (int ii=0;ii<vthid.size();ii++)
  {
    if (pthread_equal(pthread_self(),vthid[ii])) { vthid.erase(vthid.begin()+ii); break; }
  }
  pthread_spin_unlock(&vthidlock);

  pthread_cleanup_pop(1);         // 把线程清理函数出栈。
}

// 进程的退出函数。
void EXIT(int sig)  
{
  // 以下代码是为了防止信号处理函数在执行的过程中被信号中断。
  signal(SIGINT,SIG_IGN); signal(SIGTERM,SIG_IGN);

  logfile.Write("进程退出，sig=%d。\n",sig);

  TcpServer.CloseListen();    // 关闭监听的socket。

  // 取消全部的线程。
  for (int ii=0;ii<vthid.size();ii++)
  {
    pthread_cancel(vthid[ii]);
  }

  sleep(1);        // 让子线程有足够的时间退出。

  pthread_spin_destroy(&vthidlock);

  exit(0);
}

void thcleanup(void *arg)     // 线程清理函数。
{
  close((int)(long)arg);      // 关闭客户端的socket。

  logfile.Write("线程%lu退出。\n",pthread_self());
}

