/*
 *  程序名：demo47.cpp，此程序演示采用开发框架的CTcpClient类实现socket通信的客户端。
 *  作者：吴从周
*/
#include "../_public.h"
 
int main(int argc,char *argv[])
{
  CTcpClient TcpClient;   // 创建客户端的对象。
 
  if (TcpClient.ConnectToServer("172.17.0.15",5858)==false) // 向服务端发起连接请求。
  {
    printf("TcpClient.ConnectToServer(\"172.17.0.15\",5858) failed.\n"); return -1;
  }

  char strbuffer[1024];    // 存放数据的缓冲区。
 
  for (int ii=0;ii<5;ii++)   // 利用循环，与服务端进行5次交互。
  {
    memset(strbuffer,0,sizeof(strbuffer));
    snprintf(strbuffer,50,"这是第%d个超级女生，编号%03d。",ii+1,ii+1);
    printf("发送：%s\n",strbuffer);
    if (TcpClient.Write(strbuffer)==false) break;    // 向服务端发送请求报文。
 
    memset(strbuffer,0,sizeof(strbuffer));
    if (TcpClient.Read(strbuffer,20)==false) break;  // 接收服务端的回应报文。
    printf("接收：%s\n",strbuffer);
 
    sleep(1);
  }
 
  // 程序直接退出，析构函数会释放资源。
}
