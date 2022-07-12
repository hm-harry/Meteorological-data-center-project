/*
 * 程序名：demo33.cpp，此程序是网络通信的客户端程序，用于演示异步通信（poll）效率。
 * 作者：吴从周。
*/
#include "../_public.h"
 
int main(int argc,char *argv[])
{
  if (argc!=3)
  {
    printf("Using:./demo33 ip port\nExample:./demo33 192.168.174.133 5005\n\n"); return -1;
  }

  CTcpClient TcpClient;

  // 向服务端发起连接请求。
  if (TcpClient.ConnectToServer(argv[1],atoi(argv[2]))==false)
  {
    printf("TcpClient.ConnectToServer(%s,%s) failed.\n",argv[1],argv[2]); return -1;
  }

  char buffer[102400];
  int  ibuflen=0;

  CLogFile logfile(1000);
  logfile.Open("/tmp/demo33.log","a+");

  int jj=0;

  // 与服务端通讯，发送一个报文后等待回复，然后再发下一个报文。
  for (int ii=0;ii<1000000;ii++)
  {
    SPRINTF(buffer,sizeof(buffer),"这是第%d个超级女生，编号%03d。",ii+1,ii+1);
    if (TcpClient.Write(buffer)==false) break; // 向服务端发送请求报文。
    logfile.Write("发送：%s\n",buffer);

    // 接收服务端的回应报文。
    while (true)
    {
      memset(buffer,0,sizeof(buffer));
      if (TcpRead(TcpClient.m_connfd,buffer,&ibuflen,-1)==false) break;
      logfile.Write("接收：%s\n",buffer);
      jj++;
    }
  }

  // 接收服务端的回应报文。
  while (jj<1000000)
  {
    memset(buffer,0,sizeof(buffer));
    if (TcpRead(TcpClient.m_connfd,buffer,&ibuflen)==false) break;
    logfile.Write("接收：%s\n",buffer);
    jj++;
  }
}

