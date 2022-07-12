/*
 * 程序名：demo28.cpp，此程序演示基于HTTP协议的数据访问接口的简单实现。
 * 作者：吴从周
*/
#include "_public.h"
#include "_ooci.h"
 
// 从GET请求中获取参数的值：strget-GET请求报文的内容；name-参数名；value-参数值；len-参数值的长度。
bool getvalue(const char *strget,const char *name,char *value,const int len);

// 解析GET请求中的参数，从T_ZHOBTMIND1表中查询数据，返回给客户端。
bool SendData(const int sockfd,const char *strget);

int main(int argc,char *argv[])
{
  if (argc!=2)
  {
    printf("Using:./demo28 port\nExample:./demo28 8080\n\n"); return -1;
  }

  // 1、接收客户端的请求报文；
  // 2、解析URL中的参数，参数中指定了查询数据的条件；
  // 3、从T_ZHOBTMIND1表中查询数据，以xml格式返回给客户端。

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

  char strget[102400];
  memset(strget,0,sizeof(strget));

  // 接收http客户端发送过来的报文。
  recv(TcpServer.m_connfd,strget,1000,0);

  printf("%s\n",strget);

  // 先把响应报文头部发送给客户端。
  char strsend[102400];
  memset(strsend,0,sizeof(strsend));
  sprintf(strsend,\
         "HTTP/1.1 200 OK\r\n"\
         "Server: demo28\r\n"\
         "Content-Type: text/html;charset=utf-8\r\n"\
         "\r\n");
         // "Content-Length: 108909\r\n\r\n");
  if (Writen(TcpServer.m_connfd,strsend,strlen(strsend))== false) return -1;

  //logfile.Write("%s",strsend);

  // 解析GET请求中的参数，从T_ZHOBTMIND1表中查询数据，返回给客户端。
  SendData(TcpServer.m_connfd,strget);
}

// 解析GET请求中的参数，从T_ZHOBTMIND1表中查询数据，返回给客户端。
bool SendData(const int sockfd,const char *strget)
{
  // 解析URL中的参数。
  // 权限控制：用户名和密码。
  // 接口名：访问数据的种类。
  // 查询条件：设计接口的时候决定。 
  // http://127.0.0.1:8080/api?wucz&wuczpwd&getZHOBTMIND1&51076&20211024094318&20211024114020
  // http://127.0.0.1:8080/api?username=wucz&passwd=wuczpwd&intetname=getZHOBTMIND1&obtid=51076&begintime=20211024094318&endtime=20211024114020

  char username[31],passwd[31],intername[30],obtid[11],begintime[21],endtime[21];
  memset(username,0,sizeof(username));
  memset(passwd,0,sizeof(passwd));
  memset(intername,0,sizeof(intername));
  memset(obtid,0,sizeof(obtid));
  memset(begintime,0,sizeof(begintime));
  memset(endtime,0,sizeof(endtime));

  getvalue(strget,"username",username,30);    // 获取用户名。
  getvalue(strget,"passwd",passwd,30);        // 获取密码。
  getvalue(strget,"intername",intername,30);  // 获取接口名。
  getvalue(strget,"obtid",obtid,10);          // 获取站点代码。
  getvalue(strget,"begintime",begintime,20);  // 获取起始时间。
  getvalue(strget,"endtime",endtime,20);      // 获取结束时间。

  printf("username=%s\n",username);
  printf("passwd=%s\n",passwd);
  printf("intername=%s\n",intername);
  printf("obtid=%s\n",obtid);
  printf("begintime=%s\n",begintime);
  printf("endtime=%s\n",endtime);

  // 判断用户名/密码和接口名是否合法。

  // 连接数据库。
  connection conn;
  conn.connecttodb("scott/tiger@snorcl11g_132","Simplified Chinese_China.AL32UTF8");

  // 准备查询数据的SQL。
  sqlstatement stmt(&conn);
  stmt.prepare("select '<obtid>'||obtid||'</obtid>'||'<ddatetime>'||to_char(ddatetime,'yyyy-mm-dd hh24:mi:ss')||'</ddatetime>'||'<t>'||t||'</t>'||'<p>'||p||'</p>'||'<u>'||u||'</u>'||'<keyid>'||keyid||'</keyid>'||'<endl/>' from T_ZHOBTMIND1 where obtid=:1 and ddatetime>to_date(:2,'yyyymmddhh24miss') and ddatetime<to_date(:3,'yyyymmddhh24miss')");
  char strxml[1001];  // 存放SQL语句的结果集。
  stmt.bindout(1,strxml,1000);
  stmt.bindin(1,obtid,10);
  stmt.bindin(2,begintime,14);
  stmt.bindin(3,endtime,14);
 
  stmt.execute();   // 执行查询数据的SQL。

  Writen(sockfd,"<data>\n",strlen("<data>\n"));      // 返回xml的头部标签。

  while (true)
  {
    memset(strxml,0,sizeof(strxml));
    if (stmt.next()!=0) break;

    strcat(strxml,"\n");                           // 注意加上换行符。
    Writen(sockfd,strxml,strlen(strxml));          // 返回xml的每一行。
  }

  Writen(sockfd,"</data>\n",strlen("</data>\n"));  // 返回xml的尾部标签。
  
  return true;
}

// http://127.0.0.1:8080/api?username=wucz&passwd=wuczpwd&intetname=getZHOBTMIND1&obtid=51076
// 从GET请求中获取参数的值：strget-GET请求报文的内容；name-参数名；value-参数值；len-参数值的长度。
bool getvalue(const char *strget,const char *name,char *value,const int len)
{
  value[0]=0;

  char *start,*end;
  start=end=0;

  start=strstr((char *)strget,(char *)name);
  if (start==0) return false;

  end=strstr(start,"&");
  if (end==0) end=strstr(start," ");

  if (end==0) return false;

  int ilen=end-(start+strlen(name)+1);
  if (ilen>len) ilen=len;

  strncpy(value,start+strlen(name)+1,ilen);

  value[ilen]=0;

  return true;
}
