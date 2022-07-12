/*
 * 程序名：demo27.cpp，此程序用于演示HTTP客户端。
 * 作者：吴从周。
*/
#include "../_public.h"
 
int main(int argc,char *argv[])
{
  if (argc!=3)
  {
    printf("Using:./demo27 ip port\nExample:./demo27 www.weather.com.cn 80\n\n"); return -1;
  }

  CTcpClient TcpClient;

  // 向服务端发起连接请求。
  if (TcpClient.ConnectToServer(argv[1],atoi(argv[2]))==false)
  {
    printf("TcpClient.ConnectToServer(%s,%s) failed.\n",argv[1],argv[2]); return -1;
  }

  char buffer[102400];
  memset(buffer,0,sizeof(buffer));

  // 生成http请求报文。
  sprintf(buffer,\
          "GET / HTTP/1.1\r\n"\
          "Host: %s:%s\r\n"\
          "\r\n",argv[1],argv[2]);

  // 用原生的send函数把http报文发送给服务端。
  send(TcpClient.m_connfd,buffer,strlen(buffer),0);

  // 接收服务端返回的网页内容。
  while (true)
  {
    memset(buffer,0,sizeof(buffer));
    if (recv(TcpClient.m_connfd,buffer,102400,0)<=0) return 0;
    printf("%s",buffer);
  }
}

