/*
 * 程序名：demo06.cpp，此程序用于演示不粘包的socket服务端。
 * 作者：吴从周
*/
#include "../_public.h"
 
int main(int argc,char *argv[])
{
  if (argc!=2)
  {
    printf("Using:./demo06 port\nExample:./demo06 5005\n\n"); return -1;
  }

  // 第1步：创建服务端的socket。
  int listenfd;
  if ( (listenfd = socket(AF_INET,SOCK_STREAM,0))==-1) { perror("socket"); return -1; }
  
  // 第2步：把服务端用于通讯的地址和端口绑定到socket上。
  struct sockaddr_in servaddr;    // 服务端地址信息的数据结构。
  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family = AF_INET;  // 协议族，在socket编程中只能是AF_INET。
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);          // 任意ip地址。
  servaddr.sin_port = htons(atoi(argv[1]));  // 指定通讯端口。
  if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) != 0 )
  { perror("bind"); close(listenfd); return -1; }
 
  // 第3步：把socket设置为监听模式。
  if (listen(listenfd,5) != 0 ) { perror("listen"); close(listenfd); return -1; }
 
  // 第4步：接受客户端的连接。
  int  clientfd;                  // 客户端的socket。
  int  socklen=sizeof(struct sockaddr_in); // struct sockaddr_in的大小
  struct sockaddr_in clientaddr;  // 客户端的地址信息。
  clientfd=accept(listenfd,(struct sockaddr *)&clientaddr,(socklen_t*)&socklen);
  printf("客户端（%s）已连接。\n",inet_ntoa(clientaddr.sin_addr));
 
  char buffer[1024];

  // 第5步：与客户端通讯，接收客户端发过来的报文。
  while (1)
  {
    memset(buffer,0,sizeof(buffer));

    int ibuflen=0;
    if (TcpRead(clientfd,buffer,&ibuflen)==false) break; // 接收客户端的请求报文。

    printf("接收：%s\n",buffer);
  }
 
  // 第6步：关闭socket，释放资源。
  close(listenfd); close(clientfd); 
}
