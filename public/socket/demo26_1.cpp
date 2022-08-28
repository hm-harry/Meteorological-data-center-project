/*
 * 程序名：demo26.cpp，此程序演示HTTP协议，接收http请求报文。
 * 作者：吴从周
*/
#include "../_public.h"
 
// 把html文件的内容发送给客户端。
bool SendHtmlFile(const int sockfd,const char *filename);

int main(int argc,char *argv[])
{
  if (argc!=2)
  {
    printf("Using:./demo26 port\nExample:./demo26 8080\n\n"); return -1;
  }

  CTcpServer TcpServer;

  // 服务端初始化。
  if (TcpServer.InitServer(atoi(argv[1]))==false)
  {
    printf("TcpServer.InitServer(%s) failed.\n",argv[1]); return -1;
  }

  // 等待客户端的连接请求。
  if (TcpServer.Accept()==false)
  {
    printf("TcpServer.Accept() failed.\n"); return -1;
  }

  printf("客户端（%s）已连接。\n",TcpServer.GetIP());

  char buffer[102400];
  memset(buffer,0,sizeof(buffer));

  // 接收http客户端发送过来的报文。
  recv(TcpServer.m_connfd,buffer,1000,0);

  printf("%s\n",buffer);

  // 先把响应报文头部发送给客户端。
  memset(buffer,0,sizeof(buffer));
  sprintf(buffer,\
         "HTTP/1.1 200 OK\r\n"\
         "Server: demo26\r\n"\
         "Content-Type: text/html;charset=utf-8\r\n"\
         "\r\n");
         // "Content-Length: 108909\r\n\r\n");
  if (Writen(TcpServer.m_connfd,buffer,strlen(buffer))== false) return -1;

  //logfile.Write("%s",buffer);

  // 再把html文件的内容发送给客户端。
  SendHtmlFile(TcpServer.m_connfd,"SURF_ZH_20211203191856_28497.csv");
}

// 把html文件的内容发送给客户端。
bool SendHtmlFile(const int sockfd,const char *filename)
{
  int  bytes=0;
  char buffer[5000];

  FILE *fp=NULL;

  if ( (fp=FOPEN(filename,"rb")) == NULL ) return false;

  while (true)
  {
    memset(buffer,0,sizeof(buffer));

    if ((bytes=fread(buffer,1,5000,fp)) ==0) break;

    if (Writen(sockfd,buffer,bytes) == false) { fclose(fp); fp=NULL; return false; }
  }

  fclose(fp);

  return true;
}
