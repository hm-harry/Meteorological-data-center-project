/*
 * 程序名：tcpselect.cpp，此程序用于演示采用select模型的使用方法。
 * 作者：吴惠明
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

// 初始化服务端的监听端口。
int initserver(int port);

int main(int argc,char *argv[])
{
  if (argc != 2) { printf("usage: ./tcpepoll port\n"); return -1; }

  // 初始化服务端用于监听的socket。
  int listensock = initserver(atoi(argv[1]));
  printf("listensock=%d\n",listensock);

  if (listensock < 0) { printf("initserver() failed.\n"); return -1; }
// fcntl(listensock, F_SETFL, O_NONBLOCK);
fcntl(listensock, F_SETFL, fcntl(listensock, F_GETFD, 0) | O_NONBLOCK);
  // 创建epoll句柄
  int epollfd = epoll_create(1);

  // 为监听的socket准备可读事件
  struct epoll_event ev; // 声明事件的数据结构
  // ev.events = EPOLLIN; // 读事件 默认水平触发，边缘触发：ev.events = EPOLLIN|EPOLLET
  ev.events = EPOLLIN|EPOLLET;
  ev.data.fd = listensock; // 指定事件的自定义数据，会随着epoll_wait()返回的事件一起返回
  
  // 把监听的socket的事件加入epollfd中
  epoll_ctl(epollfd, EPOLL_CTL_ADD, listensock, &ev);

  struct epoll_event evs[10]; // 存放epoll返回的事件

  while (true)
  {
    // 等待监视的socket有事件发生。最后一个参数是超时时间ms
    int infds = epoll_wait(epollfd, evs, 10, -1);

    // 返回失败。
    if(infds < 0)
    {
      perror("select() failed"); break;
    }

    // 超时，在本程序中，select函数最后一个参数为空，不存在超时的情况，但以下代码还是留着。
    if (infds == 0)
    {
      printf("select() timeout.\n"); continue;
    }

    // 如果infds>0，表示有事件发送的socket的数量
    // 遍历epoll返回的已发生事件的数组
    for(int ii = 0; ii < infds; ++ii){
      printf("events = %d, data.fd = %d\n", evs[ii].events, evs[ii].data.fd);
   
      // 如果发生事件的是listensock，表示有新的客户端连上来
      if(evs[ii].data.fd == listensock){
// static int ss = 0;
// if(ss++ < 2) continue;
while(true){
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int clientsock = accept(listensock, (struct sockaddr*)&client, &len);
if(errno == EAGAIN) break;// 非阻塞的Socket没有准备好的连接
        if(clientsock < 0){ perror("accept() failed"); continue;}
fcntl(clientsock, F_SETFL, fcntl(listensock, F_GETFD, 0) | O_NONBLOCK);
        printf ("accept client(socket=%d) ok.\n",clientsock);
        // 为新客户端准备可读事件，并添加到epoll中
        ev.data.fd = clientsock;
        // ev.events = EPOLLIN;
        ev.events = EPOLLIN|EPOLLET;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock, &ev);
}
      }else{
        // 如果是客户端连接的socket有事件，表示有报文发过来或者连接已断开
        char buffer[1024]; // 存放从客户端读取的数据
        memset(buffer, 0, sizeof(buffer));
int iret = 0;
char* ptr = buffer;
while(true)
{
        if((iret = recv(evs[ii].data.fd, ptr, 10, 0)) <= 0){
if(errno == EAGAIN) break;
          // 如果客户端连接已断开
          printf("client(eventfd = %d) disconnected.\n", evs[ii].data.fd);
          close(evs[ii].data.fd); // 关闭客户端的socket
        }
        ptr = ptr + iret;
        }
        if(strlen(buffer) > 0){
          // 如果客户端有报文发过来
          printf("recv(eventfd = %d):%s\n", evs[ii].data.fd, buffer);
          // 把接收到的报文内容原封不动的发回去
          // select写机制查看是否缓冲区满，其实没必要，send自带
          send(evs[ii].data.fd, buffer, strlen(buffer), 0);
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

