/*
 * 程序名：rinetdin.cpp，网络代理服务程序-内网端。
 * 作者：吴从周
*/
#include "_public.h"

int  cmdconnsock;  // 内网程序与外网程序的控制通道。

int epollfd=0;     // epoll的句柄。
int tfd=0;         // 定时器的句柄。

#define MAXSOCK  1024
int clientsocks[MAXSOCK];       // 存放每个socket连接对端的socket的值。
int clientatime[MAXSOCK];       // 存放每个socket连接最后一次收发报文的时间。

// 向目标ip和端口发起socket连接。
int conntodst(const char *ip,const int port);

void EXIT(int sig);   // 进程退出函数。

CLogFile logfile;

CPActive PActive;     // 进程心跳。

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using :./rinetdin logfile ip port\n\n");
    printf("Sample:./rinetdin /tmp/rinetdin.log 192.168.174.132 4000\n\n");
    printf("        /project/tools1/bin/procctl 5 /project/tools1/bin/rinetdin /tmp/rinetdin.log 192.168.174.132 4000\n\n");
    printf("logfile 本程序运行的日志文件名。\n");
    printf("ip      外网代理服务端的地址。\n");
    printf("port    外网代理服务端的端口。\n\n\n");
    return -1;
  }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程。
  // 但请不要用 "kill -9 +进程号" 强行终止。
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  // 打开日志文件。
  if (logfile.Open(argv[1],"a+")==false)
  {
    printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
  }

  PActive.AddPInfo(30,"inetd");       // 设置进程的心跳超时间为30秒。

  // 建立内网程序与程序的控制通道。
  CTcpClient TcpClient;
  if (TcpClient.ConnectToServer(argv[2],atoi(argv[3]))==false)
  {
    logfile.Write("TcpClient.ConnectToServer(%s,%s) 失败。\n",argv[2],argv[3]); return -1;
  }

  cmdconnsock=TcpClient.m_connfd;
  fcntl(cmdconnsock,F_SETFL,fcntl(cmdconnsock,F_GETFD,0)|O_NONBLOCK);

  logfile.Write("与外部的控制通道已建立(cmdconnsock=%d)。\n",cmdconnsock);

  // 创建epoll句柄。
  epollfd=epoll_create(1);

  struct epoll_event ev;  // 声明事件的数据结构。

  // 为控制通道的socket准备可读事件。
  ev.events=EPOLLIN;
  ev.data.fd=cmdconnsock;
  epoll_ctl(epollfd,EPOLL_CTL_ADD,cmdconnsock,&ev);

  // 创建定时器。
  tfd=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);  // 创建timerfd。

  struct itimerspec timeout;
  memset(&timeout,0,sizeof(struct itimerspec));
  timeout.it_value.tv_sec = 20;   // 超时时间为20秒。
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(tfd,0,&timeout,NULL);  // 设置定时器。
  
  // 为定时器准备事件。
  ev.events=EPOLLIN|EPOLLET;      // 读事件，注意，一定要ET模式。
  ev.data.fd=tfd;
  epoll_ctl(epollfd,EPOLL_CTL_ADD,tfd,&ev);

  PActive.AddPInfo(30,"rinetdin");   // 设置进程的心跳超时间为30秒。

  struct epoll_event evs[10];      // 存放epoll返回的事件。

  while (true)
  {
    // 等待监视的socket有事件发生。
    int infds=epoll_wait(epollfd,evs,10,-1);

    // 返回失败。
    if (infds < 0) { logfile.Write("epoll() failed。\n"); break; }

    // 遍历epoll返回的已发生事件的数组evs。
    for (int ii=0;ii<infds;ii++)
    {
      // logfile.Write("events=%d,data.fd=%d\n",evs[ii].events,evs[ii].data.fd);

      ////////////////////////////////////////////////////////
      // 如果定时器的时间已到，设置进程的心跳，清理空闲的客户端socket。
      if (evs[ii].data.fd==tfd)
      {
        timerfd_settime(tfd,0,&timeout,NULL);  // 重新设置定时器。

        PActive.UptATime();        // 更新进程心跳。

        for (int jj=0;jj<MAXSOCK;jj++)
        {
          // 如果客户端socket空闲的时间超过80秒就关掉它。
          if ( (clientsocks[jj]>0) && ((time(0)-clientatime[jj])>80) )
          {
            logfile.Write("client(%d,%d) timeout。\n",clientsocks[jj],clientsocks[clientsocks[jj]]);
            close(clientsocks[jj]);  close(clientsocks[clientsocks[jj]]);
            // 把数组中对端的socket置空，这一行代码和下一行代码的顺序不能乱。
            clientsocks[clientsocks[jj]]=0;
            // 把数组中本端的socket置空，这一行代码和上一行代码的顺序不能乱。
            clientsocks[jj]=0;
          }
        }

        continue;
      }
      ////////////////////////////////////////////////////////

      // 如果发生事件的是控制通道。
      if (evs[ii].data.fd==cmdconnsock)
      {
        // 读取控制报文内容。
        char buffer[256];
        memset(buffer,0,sizeof(buffer));
        if (recv(cmdconnsock,buffer,200,0)<=0)
        {
          logfile.Write("与外网的控制通道已断开。\n"); EXIT(-1);
        }

        // 如果收到的是心跳报文。
        if (strcmp(buffer,"<activetest>")==0) continue;

        // 如果收到的是新建连接的命令。

        // 向外网服务端发起连接请求。
        int srcsock=conntodst(argv[2],atoi(argv[3]));
        if (srcsock<0) continue;
        if (srcsock>=MAXSOCK)
        {
          logfile.Write("连接数已超过最大值%d。\n",MAXSOCK); close(srcsock); continue;
        }

        // 从控制报文内容中获取目标服务地址和端口。
        char dstip[11];
        int  dstport;
        GetXMLBuffer(buffer,"dstip",dstip,30);
        GetXMLBuffer(buffer,"dstport",&dstport);

        // 向目标服务地址和端口发起socket连接。
        int dstsock=conntodst(dstip,dstport);
        if (dstsock<0) { close(srcsock); continue; }
        if (dstsock>=MAXSOCK)
        { 
          logfile.Write("连接数已超过最大值%d。\n",MAXSOCK); close(srcsock); close(dstsock); continue;
        } 

        // 把内网和外网的socket对接在一起。
        logfile.Write("新建内外网通道(%d,%d) ok。\n",srcsock,dstsock);

        // 为新连接的两个socket准备可读事件，并添加到epoll中。
        ev.data.fd=srcsock; ev.events=EPOLLIN;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,srcsock,&ev);
        ev.data.fd=dstsock; ev.events=EPOLLIN;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,dstsock,&ev);

        // 更新clientsocks数组中两端soccket的值和活动时间。
        clientsocks[srcsock]=dstsock; clientsocks[dstsock]=srcsock;
        clientatime[srcsock]=time(0); clientatime[dstsock]=time(0);

        continue;
      }
      ////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////
      // 以下流程处理内外网通讯链路socket的事件。
      
      char buffer[5000];   // 从socket中读到的数据。
      int  buflen=0;       // 从socket中读到的数据的大小。

      // 从一端读取数据。
      memset(buffer,0,sizeof(buffer));
      if ( (buflen=recv(evs[ii].data.fd,buffer,sizeof(buffer),0)) <= 0 )
      {
        // 如果连接已断开，需要关闭两个通道的socket。
        logfile.Write("client(%d,%d) disconnected。\n",evs[ii].data.fd,clientsocks[evs[ii].data.fd]);
        close(evs[ii].data.fd);                            // 关闭客户端的连接。
        close(clientsocks[evs[ii].data.fd]);               // 关闭客户端对端的连接。
        clientsocks[clientsocks[evs[ii].data.fd]]=0;       // 这一行代码和下一行代码的顺序不能乱。
        clientsocks[evs[ii].data.fd]=0;                    // 这一行代码和上一行代码的顺序不能乱。

        continue;
      }
      
      // 成功的读取到了数据，把接收到的报文内容原封不动的发给对端。
      // logfile.Write("from %d to %d,%d bytes。\n",evs[ii].data.fd,clientsocks[evs[ii].data.fd],buflen);
      send(clientsocks[evs[ii].data.fd],buffer,buflen,0);

      // 更新两端socket连接的活动时间。
      clientatime[evs[ii].data.fd]=time(0); 
      clientatime[clientsocks[evs[ii].data.fd]]=time(0);  
    }
  }

  return 0;
}

// 向目标ip和端口发起socket连接。
int conntodst(const char *ip,const int port)
{
  // 第1步：创建客户端的socket。
  int sockfd;
  if ( (sockfd = socket(AF_INET,SOCK_STREAM,0))==-1) return -1; 

  // 第2步：向服务器发起连接请求。
  struct hostent* h;
  if ( (h = gethostbyname(ip)) == 0 ) { close(sockfd); return -1; }
  
  struct sockaddr_in servaddr;
  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port); // 指定服务端的通讯端口。
  memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);

  // 把socket设置为非阻塞。
  fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD,0)|O_NONBLOCK);

  connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr));

  return sockfd;
}

void EXIT(int sig)
{
  logfile.Write("程序退出，sig=%d。\n\n",sig);

  // 关闭内网程序与外网程序的控制通道。
  close(cmdconnsock);

  // 关闭全部客户端的socket。
  for (int ii=0;ii<MAXSOCK;ii++)
    if (clientsocks[ii]>0) close(clientsocks[ii]);

  close(epollfd);   // 关闭epoll。

  close(tfd);       // 关闭定时器。

  exit(0);
}
