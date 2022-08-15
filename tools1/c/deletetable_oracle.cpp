/*
  程序名：deletetable_oracle.cpp，本程序是数据中心的公共功能模块，用于定时清理表中的数据
  作者：吴惠明
*/
#include "_tools_oracle.h"

#define MAXPARAMS 256

struct st_arg{
    char connstr[101];        // 数据库的连接参数
    char tname[31];           // 待清理的表名
    char keycol[31];          // 待清理的表的唯一键字段名
    char where[1001];         // 待清理数据需要满足的条件
    char starttime[31];       // 程序运行是时间区间
    int timeout;              // 本程序运行时的超时时间
    char pname[51];           // 本程序运行时的进程名
}starg;

// 显示程序的帮助
void _help(char* argv[]);

// 把xml解析到参数starg中
bool _xmltoarg(char* strxmlbuff);

CLogFile logfile;

connection conn;    // 用于执行删除SQL语句的数据库连接

// 业务处理主函数
bool _deletetable_oracle();

// 从本地表starg.localtname获取自增字段的最大值，存放在maxkeyvalue变量中
long maxkeyvalue = 0;
bool findmaxkey();

// 判断当前时间是否在程序运行的时间区间内
bool instarttime();

void EXIT(int sig);

CPActive PActive;

int main(int argc, char* argv[]){
    if (argc!=3) { _help(argv); return -1; }
    // 关闭全部的信号和输入输出，处理程序退出的信号。
    CloseIOAndSignal();
    signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

    if (logfile.Open(argv[1],"a+")==false){
        printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
    }

    // 把xml解析到参数starg结构中
    if (_xmltoarg(argv[2])==false) return -1;

    // 判断当前时间是否在程序运行的时间区间内
    if(instarttime() == false) return 0;

    PActive.AddPInfo(starg.timeout,starg.pname);
    // 注意，在调试程序的时候，可以启用类似以下的代码，防止超时。
    // PActive.AddPInfo(starg.timeout*100,starg.pname);

    if(conn.connecttodb(starg.connstr, NULL, 1) != 0){
        logfile.Write("connect database failed.\n%s\n",starg.connstr,conn.m_cda.message); EXIT(-1);
    }

    // 业务处理主函数。
    _deletetable_oracle();

    return 0;
}

// 显示程序的帮助
void _help(char* argv[]){
    printf("Using:/project/tools1/bin/deletetable_oracle logfilename xmlbuffer\n\n");

    printf("Sample:/project/tools1/bin/procctl 3600 /project/tools1/bin/deletetable_oracle /log/idc/deletetable_oracle_ZHOBTMIND1.log \"<connstr>qxidc/qxidcpwd@snorcl11g_132</connstr><tname>T_ZHOBTMIND1</tname><keycol>keyid</keycol><where>where ddatetime < sysdate - 0.03</where><starttime>01,02,03,04,05,13</starttime><timeout>120</timeout><pname>deletetable_oracle_ZHOBTMIND1</pname>\"\n\n");

    printf("本程序是数据中心的公共功能模块，用于定时清理表中的数据。\n");

    printf("logfilename  本程序运行的日志文件。\n");
    printf("xmlbuffer    本程序运行的参数。用xml表示，具体如下：\n");
    printf("connstr      数据库的连接参数，格式：ip, username, password, dbname, port。\n");
    printf("tname        待清理的表名\n");
    printf("keycol       待清理的表的唯一键字段名。\n");
    printf("starttime    程序运行是时间区间，例如02,13表示：如果程序运行是，踏中02时和13时则运行，其他时间\n"\
           "             不运行。如果starttime为空，本参数将失效，只要本程序启动就会执行数据迁移，为了减少\n"\
           "             对数据库的压力，数据迁移一般在数据库最闲的时候进行。\n");
    printf("where        同步数据的条件，填充在select remotekeycol from remotetname where remotekeycol > :1之后，\n"\
                          "注意不加where关键字。\n");
    printf("timeout      本程序的超时时间，单位：秒，视数据的大小而定，建议设置20以上。\n");
    printf("pname        进程名，尽可能采用易懂的、与其他进程不同的名称，方便故障排查。\n\n\n");
}

// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer){
  memset(&starg,0,sizeof(struct st_arg));

  // 数据库的连接参数，格式：ip, username, password, dbname, port。
  GetXMLBuffer(strxmlbuffer,"connstr",starg.connstr,100);
  if (strlen(starg.connstr)==0) { logfile.Write("connstr is null.\n"); return false; }

  // 待清理的表名。
  GetXMLBuffer(strxmlbuffer,"tname",starg.tname,30);
  if (strlen(starg.tname)==0) { logfile.Write("tname is null.\n"); return false; }

  // 待清理的表的唯一键字段名。
  GetXMLBuffer(strxmlbuffer,"keycol",starg.keycol,30);
  if (strlen(starg.keycol)==0) { logfile.Write("keycol is null.\n"); return false; }

  // 程序运行是时间区间
  GetXMLBuffer(strxmlbuffer,"starttime",starg.starttime,30);

  // 同步数据的条件，即select语句的where部分。
  GetXMLBuffer(strxmlbuffer,"where",starg.where,1000);

  // 本程序的超时时间，单位：秒，视数据量的大小而定，建议设置30以上。
  GetXMLBuffer(strxmlbuffer,"timeout",&starg.timeout);
  if (starg.timeout==0) { logfile.Write("timeout is null.\n"); return false; }

  // 本程序运行时的进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。
  GetXMLBuffer(strxmlbuffer,"pname",starg.pname,50);
  if (strlen(starg.pname)==0) { logfile.Write("pname is null.\n"); return false; }

  return true;
}

void EXIT(int sig){
  logfile.Write("程序退出，sig=%d\n\n",sig);

  conn.disconnect();

  exit(0);
}

// 业务处理主函数。
bool _deletetable_oracle(){
  CTimer Timer;
  char tmpvalue[51]; // 存放从表提取待删除记录的唯一键的值
  // 从表中提取待删除记录的唯一键
  sqlstatement stmtsel(&conn);
  stmtsel.prepare("select %s from %s %s", starg.keycol, starg.tname, starg.where);
  stmtsel.bindout(1, tmpvalue, 50);

  // 拼接绑定删除SQL语句where唯一键 in(...)的字符串
  char bindstr[2001];
  char strtemp[11];

  memset(bindstr, 0, sizeof(bindstr));

  for(int ii = 0; ii < MAXPARAMS; ++ii){// MAXPARAMS在_mysql.h中定义
    memset(strtemp, 0, sizeof(strtemp));
    sprintf(strtemp, ":%lu,", ii + 1);
    strcat(bindstr, strtemp);
  }
  bindstr[strlen(bindstr) - 1] = 0; // 最后一个逗号是多余的


  char keyvalues[MAXPARAMS][51]; // 存放唯一键字段的值

  // 准备删除数据的SQL，一次删除MAXPARAMS条记录
  sqlstatement stmtdel(&conn);
  stmtdel.prepare("delete from %s where %s in (%s)", starg.tname, starg.keycol, bindstr);
  
  for(int ii = 0; ii < MAXPARAMS; ++ii){// MAXPARAMS在_mysql.h中定义
    stmtdel.bindin(ii + 1, keyvalues[ii], 50);
  }

  int ccount = 0;
  memset(keyvalues, 0, sizeof(keyvalues));

  if(stmtsel.execute() != 0){
    logfile.Write("stmtsel.execute() failed.\n%s\n%s\n", stmtsel.m_sql,stmtsel.m_cda.message);
    return false;
  }

  while(true){
    memset(tmpvalue, 0, sizeof(tmpvalue));
    // 获取结果集
    if(stmtsel.next() != 0) break;

    strcpy(keyvalues[ccount++], tmpvalue);
    
    // 每MAXPARAMS条记录执行一次删除语句
    if(ccount == MAXPARAMS){
      if(stmtdel.execute() != 0){
        logfile.Write("stmtdel.execute() failed.\n%s\n%s\n", stmtdel.m_sql,stmtdel.m_cda.message);
        return false;
      }
      ccount = 0;
      memset(keyvalues, 0, sizeof(keyvalues));
      PActive.UptATime();
    }

  }
  // 如果不足MAXPARAMS条记录，再执行一次删除
  if(ccount > 0){
      if(stmtdel.execute() != 0){
        logfile.Write("stmtdel.execute() failed.\n%s\n%s\n", stmtdel.m_sql,stmtdel.m_cda.message);
        return false;
      }
  }
  if(stmtsel.m_cda.rpc > 0)
    logfile.Write("delete from %s %d rows in %.02fsec.\n", starg.tname, stmtsel.m_cda.rpc, Timer.Elapsed());

  return true;
}

// 判断当前时间是否在程序运行的时间区间内
bool instarttime(){
  // 程序运行的时间区间，例如02,13表示：如果程序启动时，02时和13时则运行，其他时间不运行
  if(strlen(starg.starttime) != 0){
    char strHH24[3];
    memset(strHH24, 0, sizeof(strHH24));
    LocalTime(strHH24, "hh24");
    if(strstr(starg.starttime, strHH24) == 0) return false;
  }
  return true;
}
