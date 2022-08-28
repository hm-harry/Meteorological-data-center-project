/*
 * 程序名：demo20.cpp，此程序演示采用开发框架的CTcpServer类实现socket通信多线程的服务器。
 * 作者：吴惠明
*/
#include "../_public.h"

CLogFile logfile; // 服务程序的运行日志
CTcpServer TcpServer; // 创建服务端对象
vector<pthread_t> vthid; //存放全部线程id
pthread_spinlock_t vthidlock; // 用于锁定vthid的自旋锁

void* thmain(void* arg); // 线程主函数

void EXIT(int sig); // 父进程退出函数

void thecleanup(void* arg); // 线程清理函数

int main(int argc,char *argv[])
{
  if (argc!=3)
  {
    printf("Using:./demo10 port logfile\nExample:./demo10 5005 /tmp/demo10.log\n\n"); return -1;
  }

  CloseIOAndSignal(); signal(SIGINT, EXIT); signal(SIGTERM, EXIT);

  if(logfile.Open(argv[2], "a+") == false){
    printf("logfile.Open(%s) failed\n", argv[2]);
    return -1;
  }

  // 服务端初始化
  if(TcpServer.InitServer(atoi(argv[1])) == false){
    logfile.Write("TcpServer.InitServer(%s) failed.\n", argv[1]);
    return -1;
  }
  pthread_spin_init(&vthidlock, 0);
  while(true){
    // 等待客户端的连接
    if(TcpServer.Accept() == false){
      logfile.Write("TcpServer.Accept() failed.\n");
      EXIT(-1);
      // return -1;
    }

    logfile.Write("客户端（%s）已连接。\n", TcpServer.GetIP());
// printf("listenfd = %d, connfd = %d\n", TcpServer.m_listenfd, TcpServer.m_connfd);

    // 创建一个线程，让它与客户端通讯
    pthread_t thid;
    // int* connfd = &TcpServer.m_connfd;
    if(pthread_create(&thid, NULL, thmain, (void*)(long)TcpServer.m_connfd) != 0){
      logfile.Write("pthread_create() failed.\n"); TcpServer.CloseListen(); continue;
    }
    pthread_spin_lock(&vthidlock);
    vthid.push_back(thid); // 把线程id保存到容器中
    pthread_spin_unlock(&vthidlock);
  }
}

void* thmain(void* arg){ // 线程主函数
  pthread_cleanup_push(thecleanup, arg); // 把线程清理函数入栈（关闭客户端的socket）
  int connfd = (int)(long)arg;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  pthread_detach(pthread_self());
  // 子进程与客户端进行通讯，处理业务
  char buffer[102400];
  int ibuflen;

  // 与客户端通讯，接收客户端发过来的报文后，回复ok。
  while (1)
  {
    memset(buffer,0,sizeof(buffer));
    if ( TcpRead(connfd, buffer, &ibuflen, 30) == false) break; // 接收客户端的请求报文。
    
    logfile.Write("接收：%s\n",buffer);

    strcpy(buffer,"ok");
    if ( TcpWrite(connfd, buffer) == false) break; // 向客户端发送响应结果。

    logfile.Write("发送：%s\n",buffer);
  }
  close(connfd); // 关闭客户端socker
  // 把本线程的id从容器中删除
  pthread_spin_lock(&vthidlock);
  for(int ii = 0; ii < vthid.size(); ++ii){
    if(pthread_equal(pthread_self(), vthid[ii])){
      vthid.erase(vthid.begin() + ii);
      break;
    }
  }
  pthread_spin_unlock(&vthidlock);
  pthread_cleanup_pop(1); //线程清理函数出栈
}

void EXIT(int sig){// 父进程退出函数
// 以下代码是为了防止信号处理函数在执行的过程中被信号中断
  signal(SIGINT, SIG_IGN); signal(SIGTERM, SIG_IGN);
  logfile.Write("父进程退出。sig = %d.\n", sig);
  TcpServer.CloseListen(); // 关闭监听的socket
  // 取消线程
  for(int ii = 0; ii < vthid.size(); ++ii){
    pthread_cancel(vthid[ii]);
  }
  sleep(1);
  pthread_spin_destroy(&vthidlock);
  exit(0);
}

void thecleanup(void* arg){ // 线程清理函数
  close((int)(long)arg); // 关闭客户端的socket
  logfile.Write("线程%lu退出。\n", pthread_self());
  // delete (int*)arg;
}