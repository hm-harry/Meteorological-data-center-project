/*
 *  程序名：demo48.cpp，此程序演示采用开发框架的CTcpServer类实现socket通信的服务端。
 *  作者：吴从周
*/
#include "../_public.h"
 
int main(int argc,char *argv[])
{
  CTcpServer TcpServer;   // 创建服务端对象。
 
  if (TcpServer.InitServer(5858)==false) // 初始化TcpServer的通信端口。
  {
    printf("TcpServer.InitServer(5858) failed.\n"); return -1;
  }
 
  if (TcpServer.Accept()==false)   // 等待客户端连接。
  {
    printf("TcpServer.Accept() failed.\n"); return -1;
  }
 
  printf("客户端(%s)已连接。\n",TcpServer.GetIP());
 
  char strbuffer[1024];  // 存放数据的缓冲区。
 
  while (true)
  {
    memset(strbuffer,0,sizeof(strbuffer));
    if (TcpServer.Read(strbuffer,300)==false) break; // 接收客户端发过来的请求报文。
    printf("接收：%s\n",strbuffer);
 
    strcat(strbuffer,"ok");      // 在客户端的报文后加上"ok"。
    printf("发送：%s\n",strbuffer);
    if (TcpServer.Write(strbuffer)==false) break;     // 向客户端回应报文。
  }
 
  printf("客户端已断开。\n");    // 程序直接退出，析构函数会释放资源。
}
