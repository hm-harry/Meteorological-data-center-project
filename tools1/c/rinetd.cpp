/*
 * 程序名：rinetd.cpp，网络代理服务程序-外网端
 * 作者：吴惠明
*/
#include "_public.h"

// 代理路由参数的结构体
struct st_route{
  int listenport; // 本地监听的通讯端口
  char dstip[31]; // 目标主机的ip地址
  int dstport; // 目标主机的通讯端口
  int listensock; // 本地监听的socket
}stroute;

vector<struct st_route> vroute; // 代理路由的容器
bool loadroute(const char* inifile); // 把代理路由参数加载到vroute容器中

// 初始化服务端的监听端口。
int initserver(int port);

int epollfd = 0; // 创建epoll句柄
int tfd = 0; // 创建定时器

#define MAXSOCK 1024
int clientsocks[MAXSOCK]; // 存放每个socket连接对端的socket的值
int clientatime[MAXSOCK]; // 存放每个socket连接最后一次收发报文的时间

int cmdlistensock = 0; // 服务端监听内网客户端的socket
int cmdconnsock = 0; // 内网客户端与服务端的控制通道

void EXIT(int sig); // 进程退出函数

CLogFile logfile;

CPActive PActive;

int main(int argc,char *argv[])
{
  if (argc != 4) {
    printf("\n");
    printf("usage: ./rinetd logfile inifile cmdport\n\n");
    printf("Sample: ./rinetd /tmp/rinetd.log /etc/rinetd.conf 4000\n\n");
    printf("         /project/tools/bin/procctl 5 /project/tools1/bin/rinetd /tmp/rinetd.log /etc/rinetd.conf 4000\n\n");
    printf("logfile 本程序运行的日志文件名\n");
    printf("inifile 代理服务程序参数配置文件\n");
    printf("cmdport 与内网代理程序的通讯端口\n\n");
    return -1; 
  }

  // 关闭所有的信号和输入输出
  // 设置信号，在shell状态下可用"kill + 进程号"正常终止
  // 但请不要使用"kill -9 +进程号"强行终止
  CloseIOAndSignal(); signal(SIGINT, EXIT); signal(SIGTERM, EXIT);

  // 打开日志文件
  if(logfile.Open(argv[1], "a+") == false){
    printf("打开日志文件失败（%s）。\n", argv[1]); return -1;
  }

  // 把代理路由参数加载到vroute容器
  if(loadroute(argv[2]) == false) return -1;

  logfile.Write("加载代理服务器参数成功(%d)。\n", vroute.size());

  // 初始化监听内网程序的端口，等待内网发起连接，建立控制通道
  if((cmdlistensock = initserver(atoi(argv[3]))) < 0){
    logfile.Write("initserver(%d) failed.\n", argv[3]); EXIT(-1);
  }

  // 等待内网程序的连接请求，建立控制通道
  struct sockaddr_in client;
  socklen_t len = sizeof(client);
  cmdconnsock = accept(cmdlistensock, (struct sockaddr*)&client, &len);
  if(cmdconnsock < 0){
    logfile.Write("accept() failed.\n"); EXIT(-1);
  }
  logfile.Write("与内部的控制通道已建立(cmdconnsock=%d)。\n", cmdconnsock);

  // 初始化服务端用于监听的socket。
  for(int ii = 0; ii < vroute.size(); ++ii){
    if((vroute[ii].listensock = initserver(vroute[ii].listenport)) < 0){
      logfile.Write("initserver(%d) failed.\n", vroute[ii].listenport); EXIT(-1);
    }

    // 把监听的socket设置为非阻塞
    fcntl(vroute[ii].listensock, F_SETFL, fcntl(vroute[ii].listenport, F_GETFD, 0) | O_NONBLOCK);
  }

  // 创建epoll句柄
  epollfd = epoll_create(1);

  struct epoll_event ev; // 声明事件的数据结构

  // 为监听的socket准备可读事件
  for(int ii = 0; ii < vroute.size(); ++ii){
    ev.events = EPOLLIN; // 读事件
    ev.data.fd = vroute[ii].listensock; // 指定事件的自定义数据，会随着epoll_wait()返回的事件一起返回
    epoll_ctl(epollfd, EPOLL_CTL_ADD, vroute[ii].listensock, &ev); // 把监听的socket的事件加入epollfd中
  }

  // 创建定时器
  tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC); // 创建timerfd

  struct itimerspec timeout;
  memset(&timeout, 0, sizeof(struct itimerspec));
  timeout.it_value.tv_sec = 20;
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(tfd, 0, &timeout, NULL);

  // 为定时器准备事件
  ev.events = EPOLLIN|EPOLLET; // 读事件，注意：一定要用ET模式
  ev.data.fd = tfd;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, tfd, &ev);
  
  PActive.AddPInfo(30, "rinetd"); // 设置进程的心跳超时时间为30秒

  struct epoll_event evs[10]; // 存放epoll返回的事件

  while (true)
  {
    // 等待监视的socket有事件发生。最后一个参数是超时时间ms
    int infds = epoll_wait(epollfd, evs, 10, -1);

    // 返回失败。
    if(infds < 0){ logfile.Write("epoll() failed"); break;}

    // 如果infds>0，表示有事件发送的socket的数量
    // 遍历epoll返回的已发生事件的数组
    for(int ii = 0; ii < infds; ++ii){
      /////////////////////////////////////////////////////////////////////////
      // 如果定时器的时间已到，向控制通道发送心跳，设置进程的心跳，清理空闲的socket
      if(evs[ii].data.fd == tfd){
        timerfd_settime(tfd, 0, &timeout, NULL); // 重新设置定时器

        PActive.UptATime(); // 更新进程心跳

        // 通过控制通道向内网发送心跳报文
        char buffer[256];
        strcpy(buffer, "<activetest>");
        if(send(cmdconnsock, buffer, strlen(buffer), 0) <= 0){
          logfile.Write("与内网的控制通道已经断开\n"); EXIT(-1);
        }

        for(int jj = 0; jj < MAXSOCK; ++jj){
          // 如果客户端socket空闲时间超过80秒就关闭
          if((clientsocks[jj] > 0) && ((time(0) - clientatime[jj]) > 80)){
            logfile.Write("client(%d, %d) timeout.\n", clientsocks[jj], clientsocks[clientsocks[jj]]);
            close(clientsocks[jj]); close(clientsocks[clientsocks[jj]]);
            clientsocks[clientsocks[jj]] = 0;
            clientsocks[jj] = 0;
          }
        }
        continue;
      }

      /////////////////////////////////////////////////////////////////////////
      // 如果发生事件的是监听外网的listensock，表示外网有新的客户端连上来
      int jj = 0;
      for(jj = 0; jj < vroute.size(); ++jj){
        if(evs[ii].data.fd == vroute[jj].listensock){
          // 接收外网客户端的连接
          struct sockaddr_in client;
          socklen_t len = sizeof(client);
          int srcsock = accept(vroute[jj].listensock, (struct sockaddr*)&client, &len);
          if(srcsock < 0) break;
          if(srcsock >= MAXSOCK){
            logfile.Write("连接数已超过最大值%d。\n", MAXSOCK); close(srcsock); break;
          }

          // 通过控制通道向内网程序发送命令，把路由参数传给它
          char buffer[256];
          memset(buffer, 0, sizeof(buffer));
          sprintf(buffer, "<dstip>%s</dstip><dstport>%d</dstport>", vroute[jj].dstip, vroute[jj].dstport);
          if(send(cmdconnsock, buffer, strlen(buffer), 0) <= 0){
            logfile.Write("与内网的控制通道已断开。\n"); EXIT(-1);
          }

          // 接收内网的连接
          int dstsock = accept(cmdlistensock, (struct sockaddr*)&client, &len);
          if(dstsock < 0) { close(srcsock); break;}
          if(dstsock >= MAXSOCK){
            logfile.Write("连接数已超过最大值%d。\n", MAXSOCK); close(srcsock); close(dstsock); break;
          }

          // 把内网和外网客户端的socket对接在一起

          // 为两个socket准备可读事件，并添加到epoll中
          ev.data.fd = srcsock; ev.events = EPOLLIN; epoll_ctl(epollfd, EPOLL_CTL_ADD, srcsock, &ev);
          ev.data.fd = dstsock; ev.events = EPOLLIN; epoll_ctl(epollfd, EPOLL_CTL_ADD, dstsock, &ev);

          // 更新clientsocks数组中两端socket的值和活动时间
          clientsocks[srcsock] = dstsock; clientsocks[dstsock] = srcsock;
          clientatime[srcsock] = time(0); clientatime[dstsock] = time(0);

          logfile.Write("accept port %d client(%d, %d) ok\n", vroute[jj].listenport, srcsock, dstsock);

          break;
        }
      }

      // 如果jj < vroute.size() 表示时间在上面的for循环中已经被处理
      if(jj < vroute.size()) continue;

      /////////////////////////////////////////////////////////////////////////
      // 如果是客户端连接的socket有事件，表示有报文发送过来或者连接已断开
      char buffer[5000]; // 存放从客户端读取的数据
      int buflen = 0; // 从socket中读到的数据大小

      // 从一端读取数据
      memset(buffer, 0, sizeof(buffer));
      if((buflen = recv(evs[ii].data.fd, buffer, sizeof(buffer), 0)) <= 0){
        // 如果客户端连接已断开，需要关闭两个通道
        logfile.Write("client(%d, %d) disconnected.\n", evs[ii].data.fd, clientsocks[evs[ii].data.fd]);
        close(evs[ii].data.fd); // 关闭客户端的socket
        close(clientsocks[evs[ii].data.fd]); // 关闭客户端的socket
        clientsocks[clientsocks[evs[ii].data.fd]] = 0;
        clientsocks[evs[ii].data.fd] = 0;

        continue;
      }
      
      // 成果读取到了数据，把接收到的报文原封不动地发给对端
      // logfile.Write("from %d to %d, %d bytes.\n", evs[ii].data.fd, clientsocks[evs[ii].data.fd], buflen);
      // UpdateStr(buffer, "39.107.45.57:3081", "www.weather.com.cn:80");
      // buflen = strlen(buffer);
      // logfile.Write("%s", buffer);

      send(clientsocks[evs[ii].data.fd], buffer, buflen, 0);

      // 更新两端socket连接的活动时间
      clientatime[evs[ii].data.fd] = time(0);
      clientatime[clientsocks[evs[ii].data.fd]] = time(0);
    }
  }

  return 0;
}

// 初始化服务端的监听端口。
int initserver(int port)
{
  int sock = socket(AF_INET,SOCK_STREAM,0);
  if (sock < 0)
  {
    perror("socket() failed"); return -1;
  }

  int opt = 1; unsigned int len = sizeof(opt);
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,len);

  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(sock,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
  {
    perror("bind() failed"); close(sock); return -1;
  }

  if (listen(sock,5) != 0 )
  {
    perror("listen() failed"); close(sock); return -1;
  }

  return sock;
}

// 把代理路由参数加载到vroute容器
bool loadroute(const char* inifile){
  CFile File;

  if(File.Open(inifile, "r") == false){
    logfile.Write("打开代理路由参数文件(%s)失败。\n", inifile); return false;
  }
  
  char strBuffer[256];
  CCmdStr CmdStr;

  while(true){
    memset(strBuffer, 0, sizeof(strBuffer));

    if(File.FFGETS(strBuffer, 200) == false) break;
    char* pos = strstr(strBuffer, "#");
    if(pos != 0) pos[0] = 0; // 删除说明文字
    DeleteRChar(strBuffer, ' '); // 删除右边空格
    UpdateStr(strBuffer, "   ", " ", true); // 把三个空格替换为一个空格
    CmdStr.SplitToCmd(strBuffer, " ");
    if(CmdStr.CmdCount() != 3) continue;

    memset(&stroute, 0, sizeof(struct st_route));
    CmdStr.GetValue(0, &stroute.listenport);
    CmdStr.GetValue(1,  stroute.dstip);
    CmdStr.GetValue(2, &stroute.dstport);

    vroute.push_back(stroute);
  }

  return true;
}

void EXIT(int sig){
  logfile.Write("程序退出， sig = %d \n\n", sig);

  // 关闭监听内网程序的socket
  close(cmdlistensock);

  // 关闭内网程序与外网程序的控制通道
  close(cmdconnsock);

  // 关闭全部监听的socket
  for(int ii = 0; ii < vroute.size(); ++ii){
    close(vroute[ii].listensock);
  }

  // 关闭客户端的socket
  for(int ii = 0; ii < MAXSOCK; ++ii){
    if(clientsocks[ii] > 0) close(clientsocks[ii]);
  }

  close(epollfd); // 关闭epoll句柄

  close(tfd); // 关闭定时器

  // 关闭全部客户端的socket
  exit(0);
}