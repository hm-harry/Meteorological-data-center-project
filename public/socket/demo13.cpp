/*
 * 程序名：demo13.cpp，此程序用于演示网银APP软件的客户端，增加心跳报文
 * 作者：吴惠明。
*/
#include "../_public.h"

CTcpClient TcpClient;

bool srv000(); // 心跳
bool srv001(); // 登录业务
bool srv002(); // 我的账户（查询余额）

int main(int argc,char *argv[])
{
  if (argc!=3)
  {
    printf("Using:./demo14 ip port\nExample:./demo13 127.0.0.1 5005\n\n"); return -1;
  }

  if(TcpClient.ConnectToServer(argv[1], atoi(argv[2])) == false){
    printf("TcpClient.ConnectToServer(%s, %s) failed\n", argv[1], argv[2]);
  }

  if(srv001() == false){ // 登录业务
    printf("srv001() failed.\n");
    return -1;
  }
  
  sleep(3);

  if(srv002() == false){ // 我的账户（查询余额）
    printf("srv002() failed.\n");
    return -1;
  }

  sleep(10);

  for(int ii = 3; ii < 5; ++ii){
    if(srv000() == false) break;
    sleep(ii);
  }

  if(srv002() == false){ // 我的账户（查询余额）
    printf("srv002() failed.\n");
    return -1;
  }

  return 0;
}

// 心跳
bool srv000(){
  char buffer[1024];
  SPRINTF(buffer, sizeof(buffer), "<srvcode>0</srvcode>");
  printf("发送：%s\n", buffer);
  if(TcpClient.Write(buffer) == false) return false;

  memset(buffer, 0, sizeof(buffer));
  if(TcpClient.Read(buffer) == false) return false;
  printf("接收：%s\n", buffer);

  return true;
}

// 登录业务
bool srv001(){
  char buffer[1024];

  SPRINTF(buffer, sizeof(buffer), "<srvcode>1</srvcode><tel>12312312312</tel><password>123456</password>");
  printf("发送：%s\n", buffer);
  if(TcpClient.Write(buffer) == false) return false;

  memset(buffer, 0, sizeof(buffer));
  if(TcpClient.Read(buffer) == false) return false;
  printf("接收：%s\n", buffer);

  // 解析服务端返回的xml
  int iretcode = -1;
  GetXMLBuffer(buffer, "retcode", &iretcode);
  if(iretcode != 0){
    printf("登录失败。\n");
    return false;
  }
  printf("登录成功。\n");
  return true;
}

// 我的账户（查询余额）
bool srv002(){
  char buffer[1024];

  SPRINTF(buffer, sizeof(buffer), "<srvcode>2</srvcode><cardid>6262000000001</cardid>");
  printf("发送：%s\n", buffer);
  if(TcpClient.Write(buffer) == false) return false;

  memset(buffer, 0, sizeof(buffer));
  if(TcpClient.Read(buffer) == false) return false;
  printf("接收：%s\n", buffer);

  // 解析服务端返回的xml
  int iretcode = -1;
  GetXMLBuffer(buffer, "retcode", &iretcode);
  if(iretcode != 0){
    printf("查询余额失败。\n");
    return false;
  }
  double ye = 0;
  GetXMLBuffer(buffer, "ye", &ye);
  printf("余额为(%.2f)。\n", ye);
  return true;
}

