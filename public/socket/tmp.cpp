#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>

#define MAXEVENTS 10

// 初始化服务端的监听端口。
int initserver(int port);

int main(int argc,char *argv[])
{
  if (argc != 2) { printf("usage: ./tcpepoll port\n"); return -1; }

  // 初始化服务端用于监听的socket。
  int listensock = initserver(atoi(argv[1]));
  printf("listensock=%d\n",listensock);

  if (listensock < 0) { printf("initserver() failed.\n"); return -1; }

  int epollfd=epoll_create(1);  // 创建epoll句柄。

  // 为监听的socket准备可读事件。
  struct epoll_event ev;   // 声明事件的数据结构。
  ev.events=EPOLLIN;       // 读事情。
  ev.data.fd=listensock;   // 指定事情的自定义数据，会随着epoll_wait()返回的事件一并返回。

  // 把监听的socket的事件加入epollfd中。
  epoll_ctl(epollfd,EPOLL_CTL_ADD,listensock,&ev);

  while (true)
  {
    struct epoll_event events[MAXEVENTS]; // 存放epoll返回的事件。

    // 等待监视的socket有事件发生。
    int infds=epoll_wait(epollfd,events,MAXEVENTS,-1);

    // 返回失败。
    if (infds < 0)
    {
      perror("epoll() failed"); break;
    }

    // 超时。
    if (infds == 0)
    {
      printf("epoll() timeout.\n"); continue;
    }

    // 如果infds>0，表示已发生事件发生的数量。
    // 遍历epoll返回的已发生事件的数组。
    for (int ii=0;ii<infds;ii++)
    {
      if ((events[ii].data.fd==listensock)&&(events[ii].events&EPOLLIN==1))
      {
        // 如果发生事件的是listensock，表示有新的客户端连上来。
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
        if (clientsock < 0)
        {
          printf("accept() failed.\n"); continue;
        }

        // 为新客户端准备可读事件，并添加到epoll中。
        memset(&ev,0,sizeof(struct epoll_event));
        ev.data.fd = clientsock;
        ev.events = EPOLLIN;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,clientsock,&ev);

        printf ("accept client(socket=%d) ok.\n",clientsock);
      }
      else
      {
        // 如果是客户端连接的socke有事件，表示有报文发过来或者连接已断开。
        char buffer[1024];
        memset(buffer,0,sizeof(buffer));

        // 读取客户端的数据。
        ssize_t isize=read(events[ii].data.fd,buffer,sizeof(buffer));

        // 发生了错误或socket被对方关闭。
        if (isize <=0)
        {
          printf("client(eventfd=%d) disconnected.\n",events[ii].data.fd);

          close(events[ii].data.fd);
        }
        else
        {
          // 如果客户端有报文发过来。
          printf("recv(eventfd=%d,size=%d):%s\n",events[ii].data.fd,isize,buffer);
          send(events[ii].data.fd,buffer,strlen(buffer),0);
        }
      }
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

/*
EPOLLIN     //表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
EPOLLOUT    //表示对应的文件描述符可以写；
EPOLLPRI    //表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
EPOLLERR    //表示对应的文件描述符发生错误；
EPOLLHUP    //表示对应的文件描述符被挂断；
EPOLLET     //将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
EPOLLONESHOT//只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里
*/
