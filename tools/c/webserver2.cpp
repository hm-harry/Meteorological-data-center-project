/*
 * 程序名：webserver.cpp，此程序是数据服务总线的服务端程序。
 * 采用了数据库连接池。
 * 作者：吴从周
*/
#include "_public.h"
#include "_ooci.h"

CLogFile   logfile;    // 服务程序的运行日志。
CTcpServer TcpServer;  // 创建服务端对象。

void EXIT(int sig);    // 进程的退出函数。

pthread_spinlock_t vthidlock;  // 用于锁定vthid的自旋锁。
vector<pthread_t> vthid;       // 存放全部线程id的容器。
void *thmain(void *arg);       // 线程主函数。

void thcleanup(void *arg);     // 线程清理函数。

// 主程序参数的结构体。
struct st_arg
{
  char connstr[101];  // 数据库的连接参数。
  char charset[51];   // 数据库的字符集。
  int  port;          // web服务监听的端口。
} starg;

// 显示程序的帮助
void _help(char *argv[]);

// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer);

// 读取客户端的报文。
int ReadT(const int sockfd,char *buffer,const int size,const int itimeout);

// 从GET请求中获取参数。
bool getvalue(const char *buffer,const char *name,char *value,const int len);

// 判断URL中用户名和密码，如果不正确，返回认证失败的响应报文。
bool Login(connection *conn,const char *buffer,const int sockfd);

// 判断用户是否有调用接口的权限，如果没有，返回没有权限的响应报文。
bool CheckPerm(connection *conn,const char *buffer,const int sockfd);

// 执行接口的sql语句，把数据返回给客户端。
bool ExecSQL(connection *conn,const char *buffer,const int sockfd);

#define MAXCONNS 10               // 数据库连接池的大小。 
connection conns[MAXCONNS];       // 数据库连接池的数组。
pthread_mutex_t mutex[MAXCONNS];  // 数据库连接池的锁。
bool initconns();                 // 初始数据库连接池。
connection *getconns();           // 从数据库连接池中获取一个空闲的连接。
bool freeconns(connection *conn); // 释放/归还数据库连接。
bool destroyconns();              // 释放数据库连接占用的资源（断开数据库连接，销毁锁）。

int main(int argc,char *argv[])
{
  if (argc!=3) { _help(argv); return -1; }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[1],"a+")==false) { printf("logfile.Open(%s) failed.\n",argv[1]); return -1; }

  // 把xml解析到参数starg结构中
  if (_xmltoarg(argv[2])==false) EXIT(-1);

  // 服务端初始化。
  if (TcpServer.InitServer(starg.port)==false)
  {
    logfile.Write("TcpServer.InitServer(%d) failed.\n",starg.port); EXIT(-1);
  }

  if (initconns()==false)  // 初始化数据库连接池。
  {
    logfile.Write("initconns() failed.\n"); return -1;
  }

  pthread_spin_init(&vthidlock,0);

  while (true)
  {
    // 等待客户端的连接请求。
    if (TcpServer.Accept()==false)
    {
      logfile.Write("TcpServer.Accept() failed.\n"); EXIT(-1);
    }

    logfile.Write("客户端（%s）已连接。\n",TcpServer.GetIP());

    // 创建一个新的线程，让它与客户端通讯。
    pthread_t thid;
    if (pthread_create(&thid,NULL,thmain,(void *)(long)TcpServer.m_connfd)!=0)
    {
      logfile.Write("pthread_create() failed.\n"); TcpServer.CloseListen(); continue;
    }

    pthread_spin_lock(&vthidlock);
    vthid.push_back(thid);    // 把线程id放入容器。
    pthread_spin_unlock(&vthidlock);
  }
}

void *thmain(void *arg)     // 线程主函数。
{
  pthread_cleanup_push(thcleanup,arg);       // 把线程清理函数入栈（关闭客户端的socket）。

  int connfd=(int)(long)arg;    // 客户端的socket。

  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);   // 线程取消方式为立即取消。

  pthread_detach(pthread_self());           // 把线程分离出去。

  char strrecvbuf[1024];     // 接收客户端请求报文的buffer。
  memset(strrecvbuf,0,sizeof(strrecvbuf));

  // 读取客户端的报文，如果超时或失败，线程退出。
  if (ReadT(connfd,strrecvbuf,sizeof(strrecvbuf),3)<=0) pthread_exit(0);

  // 如果不是GET请求报文不处理，线程退出。
  if (strncmp(strrecvbuf,"GET",3)!=0) pthread_exit(0);

  logfile.Write("%s\n",strrecvbuf);

  // 连接数据库。
  connection *conn=getconns();   // 获取一个数据库连接。

  // 判断URL中用户名和密码，如果不正确，返回认证失败的响应报文，线程退出。
  if (Login(conn,strrecvbuf,connfd)==false) { freeconns(conn); pthread_exit(0); }

  // 判断用户是否有调用接口的权限，如果没有，返回没有权限的响应报文，线程退出。 
  if (CheckPerm(conn,strrecvbuf,connfd)==false) { freeconns(conn); pthread_exit(0); }

  // 先把响应报文头部发送给客户端。
  char strsendbuf[1024];          
  memset(strsendbuf,0,sizeof(strsendbuf));
  sprintf(strsendbuf,\
         "HTTP/1.1 200 OK\r\n"\
         "Server: webserver\r\n"\
         "Content-Type: text/html;charset=utf-8\r\n\r\n");
  Writen(connfd,strsendbuf,strlen(strsendbuf));

  // 再执行接口的sql语句，把数据返回给客户端。
  if (ExecSQL(conn,strrecvbuf,connfd)==false) { freeconns(conn); pthread_exit(0); }

  freeconns(conn);;

  pthread_cleanup_pop(1);         // 把线程清理函数出栈。
}

// 进程的退出函数。
void EXIT(int sig)  
{
  // 以下代码是为了防止信号处理函数在执行的过程中被信号中断。
  signal(SIGINT,SIG_IGN); signal(SIGTERM,SIG_IGN);

  logfile.Write("进程退出，sig=%d。\n",sig);

  TcpServer.CloseListen();    // 关闭监听的socket。

  // 取消全部的线程。
  pthread_spin_lock(&vthidlock);
  for (int ii=0;ii<vthid.size();ii++)
  {
    // 注意，特别注意，如果线程跑得太快，主程序可能还不及把线程的id放入容器。
    // 线程清理函数可能没有来得及从容器中删除自己的id。
    // 所以，以下代码可能会出现段错误。
    pthread_cancel(vthid[ii]);
  }
  pthread_spin_unlock(&vthidlock);

  sleep(1);        // 让子线程有足够的时间退出。

  pthread_spin_destroy(&vthidlock);

  exit(0);
}

void thcleanup(void *arg)     // 线程清理函数。
{
  close((int)(long)arg);      // 关闭客户端的socket。

  // 把本线程id从存放线程id的容器中删除。
  // 把本线程id从存放线程id的容器中删除。
  // 注意，特别注意，如果线程跑得太快，主程序可能还不及把线程的id放入容器。
  // 所以，这里可能会出现找不到线程id的情况。
  pthread_spin_lock(&vthidlock);
  for (int ii=0;ii<vthid.size();ii++)
  {
    if (pthread_equal(pthread_self(),vthid[ii])) { vthid.erase(vthid.begin()+ii); break; }
  }
  pthread_spin_unlock(&vthidlock);

  logfile.Write("线程%lu退出。\n",pthread_self());
}

// 显示程序的帮助
void _help(char *argv[])
{
  printf("Using:/project/tools1/bin/webserver logfilename xmlbuffer\n\n");

  printf("Sample:/project/tools1/bin/procctl 10 /project/tools1/bin/webserver /log/idc/webserver.log \"<connstr>scott/tiger@snorcl11g_132</connstr><charset>Simplified Chinese_China.AL32UTF8</charset><port>8080</port>\"\n\n");

  printf("本程序是数据总线的服务端程序，为数据中心提供http协议的数据访问接口。\n");
  printf("logfilename 本程序运行的日志文件。\n");
  printf("xmlbuffer   本程序运行的参数，用xml表示，具体如下：\n\n");

  printf("connstr     数据库的连接参数，格式：username/password@tnsname。\n");
  printf("charset     数据库的字符集，这个参数要与数据源数据库保持一致，否则会出现中文乱码的情况。\n");
  printf("port        web服务监听的端口。\n\n");
}

// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer)
{
  memset(&starg,0,sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuffer,"connstr",starg.connstr,100);
  if (strlen(starg.connstr)==0) { logfile.Write("connstr is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"charset",starg.charset,50);
  if (strlen(starg.charset)==0) { logfile.Write("charset is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"port",&starg.port);
  if (starg.port==0) { logfile.Write("port is null.\n"); return false; }

  return true;
}

// 读取客户端的报文。
int ReadT(const int sockfd,char *buffer,const int size,const int itimeout)
{
  if (itimeout > 0)
  {
    struct pollfd fds;
    fds.fd=sockfd;
    fds.events=POLLIN;
    int iret;
    if ( (iret=poll(&fds,1,itimeout*1000)) <= 0 ) return iret;
  }

  return recv(sockfd,buffer,size,0);
}

// 判断URL中用户名和密码，如果不正确，返回认证失败的响应报文。
bool Login(connection *conn,const char *buffer,const int sockfd)
{
  char username[31],passwd[31];

  getvalue(buffer,"username",username,30); // 获取用户名。
  getvalue(buffer,"passwd",passwd,30);     // 获取密码。

  // 查询T_USERINFO表，判断用户名和密码是否存在。
  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select count(*) from T_USERINFO where username=:1 and passwd=:2 and rsts=1");
  stmt.bindin(1,username,30);
  stmt.bindin(2,passwd,30);
  int icount=0;
  stmt.bindout(1,&icount);
  stmt.execute();
  stmt.next();

  if (icount==0)   // 认证失败，返回认证失败的响应报文。
  {
    char strbuffer[256];
    memset(strbuffer,0,sizeof(strbuffer));

    sprintf(strbuffer,\
           "HTTP/1.1 200 OK\r\n"\
           "Server: webserver\r\n"\
           "Content-Type: text/html;charset=utf-8\r\n\r\n"\
           "<retcode>-1</retcode><message>username or passwd is invailed</message>");
    Writen(sockfd,strbuffer,strlen(strbuffer));

    return false;
  }

  return true;
}

// 从GET请求中获取参数。
bool getvalue(const char *buffer,const char *name,char *value,const int len)
{
  value[0]=0;

  char *start,*end;
  start=end=0;

  start=strstr((char *)buffer,(char *)name);
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

// 判断用户是否有调用接口的权限，如果没有，返回没有权限的响应报文。 
bool CheckPerm(connection *conn,const char *buffer,const int sockfd)
{
  char username[31],intername[30];

  getvalue(buffer,"username",username,30);    // 获取用户名。
  getvalue(buffer,"intername",intername,30);  // 获取接口名。

  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select count(*) from T_USERANDINTER where username=:1 and intername=:2 and intername in (select intername from T_INTERCFG where rsts=1)");
  stmt.bindin(1,username,30);
  stmt.bindin(2,intername,30);
  int icount=0;
  stmt.bindout(1,&icount);
  stmt.execute();
  stmt.next();

  if (icount!=1)
  {
    char strbuffer[256];
    memset(strbuffer,0,sizeof(strbuffer));

    sprintf(strbuffer,\
           "HTTP/1.1 200 OK\r\n"\
           "Server: webserver\r\n"\
           "Content-Type: text/html;charset=utf-8\r\n\n\n"\
           "<retcode>-1</retcode><message>permission denied</message>");

    Writen(sockfd,strbuffer,strlen(strbuffer));

    return false;
  }

  return true;
}

// 执行接口的sql语句，把数据返回给客户端。
bool ExecSQL(connection *conn,const char *buffer,const int sockfd)
{
  // 从请求报文中解析接口名。
  char intername[30];
  memset(intername,0,sizeof(intername));
  getvalue(buffer,"intername",intername,30);  // 获取接口名。

  // 从接口参数配置表T_INTERCFG中加载接口参数。
  char selectsql[1001],colstr[301],bindin[301];
  memset(selectsql,0,sizeof(selectsql)); // 接口SQL。
  memset(colstr,0,sizeof(colstr));       // 输出列名。
  memset(bindin,0,sizeof(bindin));       // 接口参数。
  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select selectsql,colstr,bindin from T_INTERCFG where intername=:1");
  stmt.bindin(1,intername,30);     // 接口名。
  stmt.bindout(1,selectsql,1000);  // 接口SQL。
  stmt.bindout(2,colstr,300);      // 输出列名。
  stmt.bindout(3,bindin,300);      // 接口参数。
  stmt.execute();  // 这里基本上不用判断返回值，出错的可能几乎没有。
  stmt.next();

  // 准备查询数据的SQL语句。
  stmt.prepare(selectsql);

  // http://192.168.174.132:8080?username=ty&passwd=typwd&intername=getzhobtmind3&obtid=59287&begintime=20211024094318&endtime=20211024113920
  // SQL语句：   select obtid,to_char(ddatetime,'yyyymmddhh24miss'),t,p,u,wd,wf,r,vis from T_ZHOBTMIND where obtid=:1 and ddatetime>=to_date(:2,'yyyymmddhh24miss') and ddatetime<=to_date(:3,'yyyymmddhh24miss')
  // colstr字段：obtid,ddatetime,t,p,u,wd,wf,r,vis
  // bindin字段：obtid,begintime,endtime

  // 绑定查询数据的SQL语句的输入变量。
  // 根据接口配置中的参数列表（bindin字段），从URL中解析出参数的值，绑定到查询数据的SQL语句中。
  //////////////////////////////////////////////////
  // 拆分输入参数bindin。
  CCmdStr CmdStr;
  CmdStr.SplitToCmd(bindin,",");

  // 声明用于存放输入参数的数组，输入参数的值不会太长，100足够。
  char invalue[CmdStr.CmdCount()][101];
  memset(invalue,0,sizeof(invalue));

  // 从http的GET请求报文中解析出输入参数，绑定到sql中。
  for (int ii=0;ii<CmdStr.CmdCount();ii++)
  {
    getvalue(buffer,CmdStr.m_vCmdStr[ii].c_str(),invalue[ii],100);
    stmt.bindin(ii+1,invalue[ii],100);
  }
  //////////////////////////////////////////////////

  // 绑定查询数据的SQL语句的输出变量。
  // 根据接口配置中的列名（colstr字段），bindout结果集。
  //////////////////////////////////////////////////
  // 拆分colstr，可以得到结果集的字段数。
  CmdStr.SplitToCmd(colstr,",");

  // 用于存放结果集的数组。
  char colvalue[CmdStr.CmdCount()][2001];

  // 把结果集绑定到colvalue数组。
  for (int ii=0;ii<CmdStr.CmdCount();ii++)
  {
    stmt.bindout(ii+1,colvalue[ii],2000);
  }
  //////////////////////////////////////////////////

  // 执行SQL语句
  char strsendbuffer[4001];  // 发送给客户端的xml。
  memset(strsendbuffer,0,sizeof(strsendbuffer));

  if (stmt.execute() != 0)
  {
    sprintf(strsendbuffer,"<retcode>%d</retcode><message>%s</message>\n",stmt.m_cda.rc,stmt.m_cda.message);
    Writen(sockfd,strsendbuffer,strlen(strsendbuffer)); 
    logfile.Write("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return false;
  }
  strcpy(strsendbuffer,"<retcode>0</retcode><message>ok</message>\n");
  Writen(sockfd,strsendbuffer,strlen(strsendbuffer)); 

  // 向客户端发送xml内容的头部标签<data>。
  Writen(sockfd,"<data>\n",strlen("<data>\n")); 

  // 获取结果集，每获取一条记录，拼接xml报文，发送给客户端。
  //////////////////////////////////////////////////
  char strtemp[2001];        // 用于拼接xml的临时变量。

  // 逐行获取结果集，发送给客户端。
  while (true)
  {
    memset(strsendbuffer,0,sizeof(strsendbuffer));
    memset(colvalue,0,sizeof(colvalue));

    if (stmt.next() != 0) break;  // 从结果集中取一条记录。

    // 拼接每个字段的xml。
    for (int ii=0;ii<CmdStr.CmdCount();ii++)
    {
      memset(strtemp,0,sizeof(strtemp));
      snprintf(strtemp,2000,"<%s>%s</%s>",CmdStr.m_vCmdStr[ii].c_str(),colvalue[ii],CmdStr.m_vCmdStr[ii].c_str());
      strcat(strsendbuffer,strtemp);
    }

    strcat(strsendbuffer,"<endl/>\n");   // xml每行结束的标志。

    Writen(sockfd,strsendbuffer,strlen(strsendbuffer));   // 向客户端返回这行数据。 
  }  
  //////////////////////////////////////////////////

  // 向客户端发送xml内容的尾部标签</data>。
  Writen(sockfd,"</data>\n",strlen("</data>\n"));

  logfile.Write("intername=%s,count=%d\n",intername,stmt.m_cda.rpc);

  // 写接口调用日志表T_USERLOG。

  return true;
}

/*
#define MAXCONNS 10               // 数据库连接池的大小。
connection conns[MAXCONNS];       // 数据库连接池的数组。
pthread_mutex_t mutex[MAXCONNS];  // 数据库连接池的锁。
bool initconns();                 // 初始数据库连接池。
connection *getconns();           // 从数据库连接池中获取一个空闲的连接。
bool freeconns(connection *conn); // 释放/归还数据库连接。
bool destroyconns();              // 释放数据库连接占用的资源（断开数据库连接，销毁锁）。
*/

bool initconns()                 // 初始数据库连接池。
{
  for (int ii=0;ii<MAXCONNS;ii++)
  {
    // 连接数据库
    if (conns[ii].connecttodb(starg.connstr,starg.charset) != 0)
    {
      logfile.Write("connect database(%s) failed.\n%s\n",starg.connstr,conns[ii].m_cda.message); 
      return false;
    }

    // 初始化互斥锁。
    pthread_mutex_init(&mutex[ii],0);
  }

  return true;
}

connection *getconns()           // 从数据库连接池中获取一个空闲的连接。
{
  while (true)
  {
    for (int ii=0;ii<MAXCONNS;ii++)
    {
      if (pthread_mutex_trylock(&mutex[ii])==0)
      {
        logfile.Write("get conns is %d.\n",ii);
        return &conns[ii];
      }
    }
    usleep(10000);   // 百分之一秒之后再重试。
  }
}

bool freeconns(connection *conn) // 释放/归还数据库连接。
{
  for (int ii=0;ii<MAXCONNS;ii++)
  {
    if (&conns[ii]==conn)
    {
      pthread_mutex_unlock(&mutex[ii]); return true;
    }
  }

  return false;
}

bool destroyconns()              // 释放数据库连接占用的资源（断开数据库连接，销毁锁）。
{
  for (int ii=0;ii<MAXCONNS;ii++)
  {
    conns[ii].disconnect();
    pthread_mutex_destroy(&mutex[ii]);
  }
 
  return true;
}











