/*
 * 程序名：webserver.cpp，此程序是数据服务总线的服务端程序。
 * 采用了线程池。
 * 作者：吴从周
*/
#include "_public.h"
#include "_ooci.h"

CLogFile   logfile;    // 服务程序的运行日志。
CTcpServer TcpServer;  // 创建服务端对象。

void EXIT(int sig);    // 进程的退出函数。

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;    // 初始化互斥锁。
pthread_cond_t  cond =PTHREAD_COND_INITIALIZER;     // 初始化条件变量。
vector<int> sockqueue;                              // 客户端socket的队列。

pthread_spinlock_t spin;       // 用于锁定vthid的自旋锁。
vector<pthread_t> vthid;       // 存放全部线程id的容器。
void *thmain(void *arg);       // 线程主函数。

void thcleanup(void *arg);     // 线程清理函数。

void *checkpool(void *arg);    // 检查数据库连接池的线程函数。

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

// 数据库连接池类。
class connpool
{
private:
  struct st_conn
  {
    connection conn;            // 数据库连接。
    pthread_mutex_t mutex;      // 用于数据库连接的互斥锁。
    time_t atime;               // 数据库连接上次使用的时间，如果未连接数据库则取值0。
  }*m_conns;                    // 数据库连接池。

  int  m_maxconns;              // 数据库连接池的最大值。
  int  m_timeout;               // 数据库连接超时时间，单位：秒。
  char m_connstr[101];          // 数据库连接参数：用户名/密码@连接名
  char m_charset[101];          // 数据库的字符集。
public:
  connpool();      // 构造函数。
 ~connpool();      // 析构函数。

  // 初始化数据库连接池，初始化锁，如果数据库连接参数有问题，返回false。
  bool init(const char *connstr,const char *charset,int maxconns,int timeout);
  // 断开数据库连接，销毁锁，释放数据库连接池的内存空间。
  void destroy();

  // 从数据库连接池中获取一个空闲的连接，成功返回数据库连接的地址。
  // 如果连接池已用完或连接数据库失败，返回空。
  connection *get();
  // 归还数据库连接。
  bool free(connection *conn);

  // 检查数据库连接池，断开空闲的连接，在服务程序中，用一个专用的子线程调用此函数。
  void checkpool();
};

connpool oraconnpool;  // 声明数据库连接池对象。

int main(int argc,char *argv[])
{
  if (argc!=3) { _help(argv); return -1; }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[1],"a+")==false) { printf("logfile.Open(%s) failed.\n",argv[1]); return -1; }

  // 把xml解析到参数starg结构中
  if (_xmltoarg(argv[2])==false) return -1;

  // 服务端初始化。
  if (TcpServer.InitServer(starg.port)==false)
  {
    logfile.Write("TcpServer.InitServer(%d) failed.\n",starg.port); return -1;
  }

  // 初始化数据库连接池。
  if (oraconnpool.init(starg.connstr,starg.charset,10,50)==false)
  {
    logfile.Write("oraconnpool.init() failed.\n"); return -1;
  }
  else
  {
    // 创建检查数据库连接池的线程。
    pthread_t thid;
    if (pthread_create(&thid,NULL,checkpool,0)!=0)
    {
      logfile.Write("pthread_create() failed.\n"); return -1;
    }
  }

  // 启动10个工作线程，线程数比CPU核数略多。
  for (int ii=0;ii<10;ii++)
  {
    pthread_t thid;
    if (pthread_create(&thid,NULL,thmain,(void *)(long)ii)!=0)
    {
      logfile.Write("pthread_create() failed.\n"); return -1;
    }

    vthid.push_back(thid);          // 把线程id保存到vthid容器中。
  }

  pthread_spin_init(&spin,0);       // 初始化给vthid加锁的自旋锁。

  while (true)
  {
    // 等待客户端的连接请求。
    if (TcpServer.Accept()==false)
    {
      logfile.Write("TcpServer.Accept() failed.\n"); return -1;
    }

    logfile.Write("客户端（%s）已连接。\n",TcpServer.GetIP());

    // 把客户端的socket放入队列，并发送条件信号。
    pthread_mutex_lock(&mutex);               // 加锁。
    sockqueue.push_back(TcpServer.m_connfd);  // 入队。
    pthread_mutex_unlock(&mutex);             // 解锁。
    pthread_cond_signal(&cond);               // 触发条件，激活一个线程。
  }
}

// 工作线程主函数。
void *thmain(void *arg)     
{
  int pthnum=(int)(long)arg;            // 线程的编号。

  pthread_cleanup_push(thcleanup,arg);  // 线程清理函数。

  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);   // 线程取消方式为立即取消。

  pthread_detach(pthread_self());       // 把线程分离出去。

  int  connfd;               // 客户端的socket。
  char strrecvbuf[1024];     // 接收客户端请求报文的buffer。
  char strsendbuf[1024];     // 向客户端发送回应报文的buffer。

  while (true)
  {
    pthread_mutex_lock(&mutex);  // 给缓存队列加锁。

    // 如果缓存队列为空，等待，用while防止条件变量虚假唤醒。
    while (sockqueue.size()==0)
    {
      pthread_cond_wait(&cond,&mutex);
    }

    // 从缓存队列中获取第一条记录，然后删除该记录。
    connfd=sockqueue[0];
    sockqueue.erase(sockqueue.begin());

    pthread_mutex_unlock(&mutex);  // 给缓存队列解锁。

    // 以下是处理业务的代码。
    logfile.Write("phid=%lu(num=%d),connfd=%d\n",pthread_self(),pthnum,connfd);

    // 读取客户端的报文，如果超时或失败，关闭客户端的socket，继续回到循环。
    memset(strrecvbuf,0,sizeof(strrecvbuf));
    if (ReadT(connfd,strrecvbuf,sizeof(strrecvbuf),3)<=0) { close(connfd); continue; }

    // 如果不是GET请求报文不处理，关闭客户端的socket，继续回到循环。
    if (strncmp(strrecvbuf,"GET",3)!=0) { close(connfd); continue; }

    logfile.Write("%s\n",strrecvbuf);

    connection *conn=oraconnpool.get();   // 获取一个数据库连接。

    // 如果数据库连接为空，向客户端返回内部错误，关闭客户端的socket，继续回到循环。
    if (conn==0)
    {
      memset(strsendbuf,0,sizeof(strsendbuf));
      sprintf(strsendbuf,\
             "HTTP/1.1 200 OK\r\n"\
             "Server: webserver\r\n"\
             "Content-Type: text/html;charset=utf-8\r\n\r\n"\
             "<retcode>-1</retcode><message>internal error.</message>");
      Writen(connfd,strsendbuf,strlen(strsendbuf));
      close(connfd); continue;
    }

    // 判断URL中用户名和密码，如果不正确，返回认证失败的响应报文，关闭客户端的socket，继续回到循环。
    if (Login(conn,strrecvbuf,connfd)==false) { oraconnpool.free(conn); close(connfd); continue; }

    // 判断用户是否有调用接口的权限，如果没有，返回没有权限的响应报文，关闭客户端的socket，继续回到循环。
    if (CheckPerm(conn,strrecvbuf,connfd)==false) { oraconnpool.free(conn); close(connfd); continue; }

    // 先把响应报文头部发送给客户端。
    memset(strsendbuf,0,sizeof(strsendbuf));
    sprintf(strsendbuf,\
           "HTTP/1.1 200 OK\r\n"\
           "Server: webserver\r\n"\
           "Content-Type: text/html;charset=utf-8\r\n\r\n");
    Writen(connfd,strsendbuf,strlen(strsendbuf));

    // 再执行接口的sql语句，把数据返回给客户端。
    if (ExecSQL(conn,strrecvbuf,connfd)==false) { oraconnpool.free(conn); close(connfd); continue; }

    oraconnpool.free(conn);;
  }

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
  pthread_spin_lock(&spin);
  for (int ii=0;ii<vthid.size();ii++)
  {
    pthread_cancel(vthid[ii]);
  }
  pthread_spin_unlock(&spin);

  sleep(1);        // 让子线程有足够的时间退出。

  pthread_spin_destroy(&spin);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);

  exit(0);
}

void thcleanup(void *arg)     // 线程清理函数。
{
  pthread_mutex_unlock(&mutex);

  // 把本线程id从存放线程id的容器中删除。
  pthread_spin_lock(&spin);
  for (int ii=0;ii<vthid.size();ii++)
  {
    if (pthread_equal(pthread_self(),vthid[ii])) { vthid.erase(vthid.begin()+ii); break; }
  }
  pthread_spin_unlock(&spin);

  logfile.Write("线程%d(%lu)退出。\n",(int)(long)arg,pthread_self());
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

connpool::connpool()
{
  m_maxconns=0;
  m_timeout=0;
  memset(m_connstr,0,sizeof(m_connstr));
  memset(m_charset,0,sizeof(m_charset));
  m_conns=0;
}

// 初始化数据库连接池，初始化锁，如果数据库连接参数有问题，返回false。
bool connpool::init(const char *connstr,const char *charset,const int maxconns,int timeout)     
{
  // 尝试数据库，验证数据库连接参数是否正确。
  connection conn;
  if (conn.connecttodb(connstr,charset)!=0)
  {
    printf("连接数据库失败。\n%s\n",conn.m_cda.message); return false;
  }
  conn.disconnect();

  strncpy(m_connstr,connstr,100);
  strncpy(m_charset,charset,100);
  m_maxconns=maxconns;
  m_timeout=timeout;

  // 分配数据库连接池内存空间。
  m_conns=new struct st_conn[m_maxconns];

  for (int ii=0;ii<m_maxconns;ii++)
  {
    pthread_mutex_init(&m_conns[ii].mutex,0);    // 初始化锁。
    m_conns[ii].atime=0;                         // 数据库连接上次使用的时间初初化为0。
  }

  return true;
}

connpool::~connpool()
{
  destroy();
}

// 断开数据库连接，销毁锁，释放连接池。
void connpool::destroy()
{
  for (int ii=0;ii<m_maxconns;ii++)
  {
    m_conns[ii].conn.disconnect();             // 断开数据库连接。
    pthread_mutex_destroy(&m_conns[ii].mutex); // 断开数据库连接。
  }

  delete []m_conns;         // 释放数据库连接池的内存空间。
  m_conns=0;

  memset(m_connstr,0,sizeof(m_connstr));
  memset(m_charset,0,sizeof(m_charset));
  m_maxconns=0;
  m_timeout=0;
}

// 1)从数据库连接池中寻找一个空闲的、已连接好的connection，如果找到了，返回它的地址。
// 2)如果没有找到，在连接池中找一个未连接数据库的connection，连接数据库，如果成功，返回connection的地址。
// 3)如果第2)步找到了未连接数据库的connection，但是，连接数据库失败，返回空。
// 4)如果第2)步没有找到未连接数据库的connection，表示数据库连接池已用完，也返回空。
connection *connpool::get()
{
  int pos=-1;       // 用于记录第一个未连接数据库的数组位置。

  for (int ii=0;ii<m_maxconns;ii++)
  {
    if (pthread_mutex_trylock(&m_conns[ii].mutex)==0)       // 尝试加锁。
    {
      if (m_conns[ii].atime>0)       // 如果数据库连接是已连接的状态。
      {
        printf("取到连接%d。\n",ii); 
        m_conns[ii].atime=time(0);   // 把数据库连接的使用时间设置为当前时间。
        return &m_conns[ii].conn;    // 返回数据库连接的地址。
      }

      if (pos==-1) pos=ii;          // 记录第一个未连接数据库的数组位置。
      else pthread_mutex_unlock(&m_conns[ii].mutex);     // 释放锁。
    }
  }

  if (pos==-1)   // 如果连接池已用完，返回空。
  {
    printf("连接池已用完。\n"); return NULL;
  }

  // 连接池没有用完，让m_conns[pos].conn连上数据库。
  printf("新连接%d。\n",pos);

  // 连接数据库。
  if (m_conns[pos].conn.connecttodb(m_connstr,m_charset)!=0)
  {
    // 如果连接数据库失败，释放锁，返回空。
    printf("连接数据库失败。\n");
    pthread_mutex_unlock(&m_conns[pos].mutex);  // 释放锁。
    return NULL;
  }
  
  m_conns[pos].atime=time(0);      // 把数据库连接的使用时间设置为当前时间。

  return &m_conns[pos].conn;
}

// 归还数据库连接。 
bool connpool::free(connection *conn) 
{
  for (int ii=0;ii<m_maxconns;ii++)
  {
    if (&m_conns[ii].conn==conn)
    {
      printf("归还%d\n",ii);
      m_conns[ii].atime=time(0);      // 把数据库连接的使用时间设置为当前时间。
      pthread_mutex_unlock(&m_conns[ii].mutex);        // 释放锁。
      return true;
    }
  }

  return false;
}

// 检查连接池，断开空闲的连接。
void connpool::checkpool()
{
  for (int ii=0;ii<m_maxconns;ii++)
  {
    if (pthread_mutex_trylock(&m_conns[ii].mutex)==0)  // 尝试加锁。
    {
      if (m_conns[ii].atime>0)    // 如果是一个可用的连接。
      {
        // 判断连接是否超时。
        if ( (time(0)-m_conns[ii].atime)>m_timeout ) 
        {
          printf("连接%d已超时。\n",ii);
          m_conns[ii].conn.disconnect();     // 断开数据库连接。
          m_conns[ii].atime=0;               // 重置数据库连接的使用时间。
        }
        else  
        {
          // 如果没有超时，执行一次sql，验证连接是否有效，如果无效，断开它。
          // 如果网络断开了，或者数据库重启了，那么就需要重连数据库，在这里，只需要断开连接就行了，
          // 重连的操作交给get()函数。
          if (m_conns[ii].conn.execute("select * from dual")!=0)
          {
            printf("连接%d已故障。\n",ii);
            m_conns[ii].conn.disconnect();     // 断开数据库连接。
            m_conns[ii].atime=0;               // 重置数据库连接的使用时间。
          }
        }
      }

      pthread_mutex_unlock(&m_conns[ii].mutex);   // 释放锁。
    }
    
    // 如果尝试加锁失败，表示数据库连接正在使用中，不必检查。
  }
}

void *checkpool(void *arg)    // 检查数据库连接池的线程函数。
{
  while (true)
  {
    oraconnpool.checkpool();  sleep(30);
  }
}


