/*
  程序名：migratetable.cpp，本程序是数据中心的公共功能模块，用于迁移表中的数据
  作者：吴惠明
*/
#include "_tools.h"

struct st_arg{
    char connstr[101];        // 数据库的连接参数
    char srctname[31];        // 待迁移数据表的表名
    char dsttname[31];        // 迁移目的表的表名，注意，srctname和dsttname的结构必须完全相同。
    char keycol[31];          // 待清理的表的唯一键字段名
    char where[1001];         // 待清理数据需要满足的条件
    char starttime[31];       // 程序运行是时间区间
    int maxcount;             // 每执行一次迁移操作的记录数据，不能超过MAXPARAMS（256）
    int timeout;              // 本程序运行时的超时时间
    char pname[51];           // 本程序运行时的进程名
}starg;

// 显示程序的帮助
void _help(char* argv[]);

// 把xml解析到参数starg中
bool _xmltoarg(char* strxmlbuff);

CLogFile logfile;

connection conn1;    // 用于查询SQL语句的数据库连接
connection conn2;    // 用于执行删除SQL语句的数据库连接

// 业务处理主函数
bool _migratetable();

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

    if(conn1.connecttodb(starg.connstr, NULL) != 0){
        logfile.Write("connect database failed.\n%s\n",starg.connstr,conn1.m_cda.message); EXIT(-1);
    }

    if(conn2.connecttodb(starg.connstr, NULL) != 0){
      logfile.Write("connect database failed.\n%s\n%s\n", starg.connstr, conn2.m_cda.message);
      EXIT(-1);
    }

    // 业务处理主函数。
    _migratetable();

    return 0;
}

// 显示程序的帮助
void _help(char* argv[]){
    printf("Using:/project/tools1/bin/migratetable logfilename xmlbuffer\n\n");

    printf("Sample:/project/tools1/bin/procctl 3600 /project/tools1/bin/migratetable /log/idc/migratetable_ZHOBTMIND.log \"<connstr>172.29.193.250,root,whmhhh1998818,mysql,3306</connstr><srctname>T_ZHOBTMIND</srctname><dsttname>T_ZHOBTMIND_HIS</dsttname><keycol>keyid</keycol><where>where ddatetime < timestampadd(minute, -120, now())</where><starttime>01,02,03,04,05,13</starttime><maxcount>300</maxcount><timeout>120</timeout><pname>migratetable_ZHOBTMIND</pname>\"\n\n");

    printf("本程序是数据中心的公共功能模块，用于迁移表中的数据。\n");

    printf("logfilename  本程序运行的日志文件。\n");
    printf("xmlbuffer    本程序运行的参数。用xml表示，具体如下：\n");
    printf("connstr      数据库的连接参数，格式：ip, username, password, dbname, port。\n");
    printf("srctname     待迁移数据表的表名\n");
    printf("dsttname     迁移目的表的表名，注意，srctname和dsttname的结构必须完全相同。\n");
    printf("keycol       待清理的表的唯一键字段名。\n");
    printf("starttime    程序运行是时间区间，例如02,13表示：如果程序运行是，踏中02时和13时则运行，其他时间\n"\
           "             不运行。如果starttime为空，本参数将失效，只要本程序启动就会执行数据迁移，为了减少\n"\
           "             对数据库的压力，数据迁移一般在数据库最闲的时候进行。\n");
    printf("where        同步数据的条件，填充在select remotekeycol from remotetname where remotekeycol > :1之后，\n"\
                          "注意不加where关键字。\n");
    printf("maxcount     每执行一次迁移操作的记录数据，不能超过MAXPARAMS（256）\n");
    printf("timeout      本程序的超时时间，单位：秒，视数据的大小而定，建议设置120以上。\n");
    printf("pname        进程名，尽可能采用易懂的、与其他进程不同的名称，方便故障排查。\n\n\n");
}

// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer){
  memset(&starg,0,sizeof(struct st_arg));

  // 数据库的连接参数，格式：ip, username, password, dbname, port。
  GetXMLBuffer(strxmlbuffer,"connstr",starg.connstr,100);
  if (strlen(starg.connstr)==0) { logfile.Write("connstr is null.\n"); return false; }

  // 待迁移数据表的表名。
  GetXMLBuffer(strxmlbuffer,"srctname",starg.srctname,30);
  if (strlen(starg.srctname)==0) { logfile.Write("srctname is null.\n"); return false; }

  // 迁移目的表的表名，注意，srctname和dsttname的结构必须完全相同。
  GetXMLBuffer(strxmlbuffer,"dsttname",starg.dsttname,30);
  if (strlen(starg.dsttname)==0) { logfile.Write("dsttname is null.\n"); return false; }

  // 待清理的表的唯一键字段名。
  GetXMLBuffer(strxmlbuffer,"keycol",starg.keycol,30);
  if (strlen(starg.keycol)==0) { logfile.Write("keycol is null.\n"); return false; }

  // 程序运行是时间区间
  GetXMLBuffer(strxmlbuffer,"starttime",starg.starttime,30);

  // 同步数据的条件，即select语句的where部分。
  GetXMLBuffer(strxmlbuffer,"where",starg.where,1000);

  // 每执行一次迁移操作的记录数据，不能超过MAXPARAMS（256）
  GetXMLBuffer(strxmlbuffer,"maxcount",&starg.maxcount);
  if(starg.maxcount > MAXPARAMS) starg.maxcount = MAXPARAMS;

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

  conn1.disconnect();

  conn2.disconnect();

  exit(0);
}

// 业务处理主函数。
bool _migratetable(){
  CTimer Timer;

  // 从数据字典中获取表的全部字段名
  CTABCOLS TABCOLS;

  // 获取迁移数据表的字段名，用starg.srctname和starg.dsttname都可以
  if(TABCOLS.allcols(&conn2, starg.dsttname) == false){
    logfile.Write("表%s不存在。\n", starg.dsttname);
    return false;
  }

  char tmpvalue[51]; // 存放从表提取待删除记录的唯一键的值

  // 从表中提取待删除记录的唯一键
  sqlstatement stmtsel(&conn1);
  stmtsel.prepare("select %s from %s %s", starg.keycol, starg.srctname, starg.where);
  stmtsel.bindout(1, tmpvalue, 50);

  // 拼接绑定删除SQL语句where唯一键 in(...)的字符串
  char bindstr[2001];
  char strtemp[11];

  memset(bindstr, 0, sizeof(bindstr));

  for(int ii = 0; ii < starg.maxcount; ++ii){// MAXPARAMS在_mysql.h中定义
    memset(strtemp, 0, sizeof(strtemp));
    sprintf(strtemp, ":%lu,", ii + 1);
    strcat(bindstr, strtemp);
  }
  bindstr[strlen(bindstr) - 1] = 0; // 最后一个逗号是多余的


  char keyvalues[starg.maxcount][51]; // 存放唯一键字段的值

  // 准备插入和删除数据的SQL，一次删除starg.maxcount条记录
  sqlstatement stmtins(&conn2);
  stmtins.prepare("insert into %s(%s) select %s from %s where %s in (%s)", starg.dsttname, TABCOLS.m_allcols, TABCOLS.m_allcols, starg.srctname, starg.keycol, bindstr);

  sqlstatement stmtdel(&conn2);
  stmtdel.prepare("delete from %s where %s in (%s)", starg.srctname, starg.keycol, bindstr);
  
  for(int ii = 0; ii < starg.maxcount; ++ii){// starg.maxcount在_mysql.h中定义
    stmtins.bindin(ii + 1, keyvalues[ii], 50);
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
    
    // 每starg.maxcount条记录执行一次删除语句
    if(ccount == starg.maxcount){
      // 先插入starg.dsttname表
      if(stmtins.execute() != 0){
        logfile.Write("stmtins.execute() failed.\n%s\n%s\n", stmtins.m_sql,stmtins.m_cda.message);
        if(stmtins.m_cda.rc != 1062) return false;
      }

      // 先删除starg.srctname表
      if(stmtdel.execute() != 0){
        logfile.Write("stmtdel.execute() failed.\n%s\n%s\n", stmtdel.m_sql,stmtdel.m_cda.message);
        return false;
      }

      conn2.commit();
      ccount = 0;
      memset(keyvalues, 0, sizeof(keyvalues));
      PActive.UptATime();
    }

  }
  // 如果不足MAXPARAMS条记录，再执行一次删除
  if(ccount > 0){
      // 先插入starg.dsttname表
      if(stmtins.execute() != 0){
        logfile.Write("stmtins.execute() failed.\n%s\n%s\n", stmtins.m_sql,stmtins.m_cda.message);
        if(stmtins.m_cda.rc != 1062) return false;
      }

      // 先删除starg.srctname表
      if(stmtdel.execute() != 0){
        logfile.Write("stmtdel.execute() failed.\n%s\n%s\n", stmtdel.m_sql,stmtdel.m_cda.message);
        return false;
      }
      conn2.commit();
  }
  if(stmtsel.m_cda.rpc > 0)
    logfile.Write("migrate from %s to %s %d rows in %.02fsec.\n", starg.srctname, starg.dsttname, stmtsel.m_cda.rpc, Timer.Elapsed());

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