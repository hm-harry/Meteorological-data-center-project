/*
  程序名：dminingmysql.cpp，本程序是数据中心的公共功能模块，用于从mysql数据库源表抽取数据，生成xml文件
  作者：吴惠明
*/
#include"_public.h"
#include"_mysql.h"

#define MAXFIELDCOUNT 100 // 结果集字段的最大数
int MAXFIELDLEN = -1;   // 结果集字段值的最大长度

struct st_arg{
  char connstr[101];        // 数据库连接参数
  char charset[51];         // 数据库字符集
  char selectsql[1024];     // 从数据源数据库抽取数据的SQL语句
  char fieldstr[501];       // 抽取数据的SQL语句输出结果集字段名，字段名之间用逗号隔开
  char fieldlen[501];       // 抽取数据的SQL语句输出结果集字段的长度，用逗号分割
  char bfilename[31];       // 输出xml文件的前缀
  char efilename[31];       // 输出xml文件的后缀
  char outpath[301];        // 输出xml文件存放的目录
  int maxcount;             // 输出xml文件最大记录数，0表示无限制
  char starttime[51];       // 程序运行的时间区间
  char incfield[31];        // 递增字段名
  char incfilename[301];    // 已抽取数据的递增字段最大值存放的文件
  char connstr1[101];       // 已抽取数据的递增字段最大值存放的数据库的连接参数
  int timeout;              // 进程心跳的超时时间
  char pname[51];           // 进程名，建议使用"dminingmysql_后缀"的方式
}starg;

CLogFile logfile;

CPActive PActive; // 进程心跳

connection conn, conn1;

char strfieldname[MAXFIELDCOUNT][31];   // 结果集字段名数组，从starg.fieldstr解析得到
int ifieldlen[MAXFIELDCOUNT];           // 结果集字段的长度数组，从starg.fieldlen解析得到
int ifieldcount;                        // strfieldname和ifieldlen数组中的有效字段个数
int incfieldpos = -1;                   // 递增字段在结果集数组中的位置

char strxmlfilename[301];  // xml文件名

long imaxincvalue; // 自增字段最大值

// 程序退出和信号2/5的处理函数
void EXIT(int sig);

void _help();

// 把xml解析到参数starg结构体中
bool _xmltoarg(char *strxmlbuff);

// 上传文件功能的主函数
bool _dminingmysql();

// 判断当前时间是否在程序运行的时间区间内
bool instarttime();

// 生成xml文件名
void crtxmlfilename();

// 从数据库表或starg.incfilename文件中获取已抽取数据的最大id
bool readincfield();

// 把已抽取数据的最大id写入数据库表或starg.incfilename文件
bool writeincfield();

int main(int argc, char* argv[]){
  // 把服务器上某个目录的文件全部上传到本地目录（可以指定文件名匹配规则）。
  if(argc != 3){
    _help();
    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号，在shell状态下可用"kill + 进程号"正常终止进程
  // 但请不要用"kill -9 + 进程号"强行终止
  // CloseIOAndSignal();
  signal(SIGINT, EXIT);
  signal(SIGTERM, EXIT);

  // 打开日志文件
  if(logfile.Open(argv[1], "a+") == false){
    printf("打开日志文件失败（%s）\n", argv[1]);
    return -1;
  }

  // 解析xml，得到程序运行的参数
  if(_xmltoarg(argv[2]) == false) return -1;

  // 判断当前时间是否在程序运行的时间区间内
  if(instarttime() == false) return 0;

  PActive.AddPInfo(starg.timeout, starg.pname); // 把进程的心跳信息写入共享内存

  // 连接数据库
  if(conn.connecttodb(starg.connstr, starg.charset) != 0){
    logfile.Write("connect database(%s) failed.\n%s\n", starg.connstr, conn.m_cda.message);
    return -1;
  }
  logfile.Write("connect database(%s) ok.\n", starg.connstr);

  // 连接本地的数据库，用于存放已抽取数据的自增字段最大值
  if(strlen(starg.connstr1) != 0){
    if(conn1.connecttodb(starg.connstr1, starg.charset) != 0){
      logfile.Write("connect database(%s) failed.\n%s\n", starg.connstr1, conn1.m_cda.message);
      return -1;
    }
    logfile.Write("connect database(%s) ok.\n", starg.connstr1);
  }
  
  // 上传文件功能的主函数
  _dminingmysql();
  return 0;
}

void EXIT(int sig){
  printf("进程退出，sig = %d\n\n", sig);

  exit(0);
}

void _help(){
    printf("\n");
    printf("Using:/project/tools1/bin/dminingmysql logfilename xmlbuffer\n\n");

    printf("Sample:/project/tools1/bin/procctl 3600 /project/tools1/bin/dminingmysql /log/idc/dminingmysql_ZHOBTCODE.log \
    \"<connstr>127.0.0.1,root,whmhhh1998818,mysql,3306</connstr><charset>utf8</charset><selectsql>Select obtid,cityname,provname,lat,lon,height from T_ZHOBTCODE</selectsql>\
    <fieldstr>obtid,cityname,provname,lat,lon,height</fieldstr><fieldlen>10,30,30,10,10,10</fieldlen><bfilename>ZHOBTCODE</bfilename>\
    <efilename>HYCZ</efilename><outpath>/idcdata/dmindata</outpath><timeout>30</timeout><pname>dminingmysql_ZHOBTCODE</pname>\"\n\n");

    printf("     /project/tools1/bin/procctl 30 /project/tools1/bin/dminingmysql /log/idc/dminingmysql_ZHOBTMIND.log \
    \"<connstr>127.0.0.1,root,whmhhh1998818,mysql,3306</connstr><charset>utf8</charset>\
    <selectsql>Select obtid,date_format(ddatetime,'%%%%Y-%%%%m-%%%%d %%%%H:%%%%m:%%%%s'),t,p,u,wd,wf,r,vis,keyid from T_ZHOBTMIND where keyid>:1 and ddatetime>timestampadd(minute,-120,now())</selectsql>\
    <fieldstr>obtid,ddatetime,t,p,u,wd,wf,r,vis,keyid</fieldstr><fieldlen>10,19,8,8,8,8,8,8,8,15</fieldlen><bfilename>ZHOBTMIND</bfilename>\
    <efilename>HYCZ</efilename><outpath>/idcdata/dmindata</outpath><starttime></starttime><incfield>keyid</incfield><incfilename>/idcdata/dmining/dminingmysql_ZHOBTMIND_HYCZ.list</incfilename>\
    <timeout>30</timeout><pname>dminingmysql_ZHOBTMIND_HYCZ</pname><maxcount>1000</maxcount><connstr1>127.0.0.1,root,whmhhh1998818,mysql,3306</connstr1>\"\n\n");

    printf("本程序是数据中心的公共功能模块，用于从mysql数据库源表抽取数据，生成xml文件\n");
    printf("logfilename     是本程序的运行的日志文件。\n");
    printf("xmlbuffer       为文件上传的参数，如下：\n");
    printf("connstr         数据库连接参数,格式：ip,username,password,dbname,port。\n");
    printf("charset         数据库的字符集，这个参数要与数据源数据库保持一致，否则会出现中文乱码的现象\n");
    printf("selectsql       从数据源数据库抽取数据的SQL语句，注意：1.时间函数的百分号%需要四个，显示出来才有两个，被prepare后将剩一个\n");
    printf("fieldstr        抽取数据的SQL语句输出结果集字段名，字段名之间用逗号隔开，作为xml文件的字段名\n");
    printf("fieldlen        抽取数据的SQL语句输出结果集字段的长度，中间用逗号隔开。fieldstr与fieldlen的字段必须意义对应\n");
    printf("bfilename       输出xml文件的前缀。\n");
    printf("efilename       输出xml文件的后缀。\n");
    printf("maxcount        输出xml文件的最大记录数，缺省值为0，表示无限制。\n");
    printf("outpath         输出xml文件存放的目录。\n");
    printf("starttime       程序运行的时间区间，例如02,13表示：如果程序启动时，02时和13时则运行，其他时间不运行"\
    "如果starttime为空，那么starttime参数将失效，只要本程序启动就会执行数据抽取，为了减少数据源的压力，从数据库抽取的时候，一般在对方数据库最闲的时候进行。\n");
    printf("incfilename     已抽取数据的递增字段放最大值存放的文件，如果该文件丢失，将重挖全部的数据\n");
    printf("connstr1        已抽取数据的递增字段最大值存放的数据库连接参数。connstr1和incfilename二选一，connstr1优先。\n");
    printf("timeout         本程序的超时时间，单位：秒。\n");
    printf("pname           进程名，尽可能采用易懂的、与其他进程不同的名称，方便故障排查。\n\n\n");

}

bool _xmltoarg(char *strxmlbuff){
  memset(&starg, 0, sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuff, "connstr", starg.connstr, 100);
  if(strlen(starg.connstr) == 0){
    logfile.Write("connstr is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "charset", starg.charset, 50);
  if(strlen(starg.charset) == 0){
    logfile.Write("charset is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "selectsql", starg.selectsql, 1000);
  if(strlen(starg.selectsql) == 0){
    logfile.Write("selectsql is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "fieldstr", starg.fieldstr, 500);
  if(strlen(starg.fieldstr) == 0){
    logfile.Write("fieldstr is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "fieldlen", starg.fieldlen, 500);
  if(strlen(starg.fieldlen) == 0){
    logfile.Write("fieldlen is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "bfilename", starg.bfilename, 30);
  if(strlen(starg.bfilename) == 0){
    logfile.Write("bfilename is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "efilename", starg.efilename, 30);
  if(strlen(starg.efilename) == 0){
    logfile.Write("efilename is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "outpath", starg.outpath, 300);
  if(strlen(starg.outpath) == 0){
    logfile.Write("outpath is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "starttime", starg.starttime, 50); // 可选参数

  GetXMLBuffer(strxmlbuff, "incfield", starg.incfield, 30); // 可选参数

  GetXMLBuffer(strxmlbuff, "incfilename", starg.incfilename, 300); // 可选参数

  GetXMLBuffer(strxmlbuff, "maxcount", &starg.maxcount); // 可选参数

  GetXMLBuffer(strxmlbuff, "connstr1", starg.connstr1, 100); // 可选参数

  GetXMLBuffer(strxmlbuff, "timeout", &starg.timeout);
  if(starg.timeout == 0){
    logfile.Write("timeout is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "pname", starg.pname, 50);
  if(strlen(starg.pname) == 0){
    logfile.Write("pname is null.\n");
    return false;
  }

  CCmdStr CmdStr;

  // 把starg.fieldlen解析到ifieldlen数组中
  CmdStr.SplitToCmd(starg.fieldlen, ",");

  // 判断字段数是否超出MAXFIELDCOUNT的限制
  if(CmdStr.CmdCount() > MAXFIELDCOUNT){
    logfile.Write("fieldlen的字段数太多，超出了最大限制%d。\n", MAXFIELDCOUNT);
    return false;
  }

  for(int ii = 0; ii < CmdStr.CmdCount(); ++ii){
    CmdStr.GetValue(ii, &ifieldlen[ii]);
    if(ifieldlen[ii] > MAXFIELDLEN) MAXFIELDLEN = ifieldlen[ii]; // 字段的长度不能超过MAXFIELDLEN
  }
  ifieldcount = CmdStr.CmdCount();

  // 把starg.fieldstr解析到strfieldname数组中
  CmdStr.SplitToCmd(starg.fieldstr, ",");

  // 判断字段数是否超出MAXFIELDCOUNT的限制
  if(CmdStr.CmdCount() > MAXFIELDCOUNT){
    logfile.Write("fieldstr的字段数太多，超出了最大限制%d。\n", MAXFIELDCOUNT);
    return false;
  }

  for(int ii = 0; ii < CmdStr.CmdCount(); ++ii){
    CmdStr.GetValue(ii, strfieldname[ii], 30);
  }

  // 判断fieldlen和fieldstr两个数组中的字段是否一致
  if(ifieldcount != CmdStr.CmdCount()){
    logfile.Write("fieldlen和fieldstr两个数组中的字段不一致。\n");
    return false;
  }

  // 获取自增字段在结果集中的位置
  if(strlen(starg.incfield) != 0){
    for(int ii = 0; ii < ifieldcount; ++ii){
      if(strcmp(starg.incfield, strfieldname[ii]) == 0){
        incfieldpos = ii;
        break;
      }
    }
    if(incfieldpos == -1){
      logfile.Write("递增字段%s不在列表%s中。\n",starg.incfield, starg.fieldstr);
      return false;
    }
    if((strlen(starg.incfilename) == 0) && (strlen(starg.connstr1) == 0)){
      logfile.Write("incfilename和connstr1参数必须二选一。\n");
      return false;
    }
  }

  return true;
}

// 上传文件功能的主函数
bool  _dminingmysql(){
  // 从starg.incfilename文件中获取已抽取数据的最大id
  readincfield();

  sqlstatement stmt(&conn);
  stmt.prepare(starg.selectsql);
  char strfieldvalue[ifieldcount][MAXFIELDLEN + 1];// 抽取数据的SQL执行后存放结果集字段值的数组
  for(int ii = 1; ii <= ifieldcount; ++ii){
    stmt.bindout(ii, strfieldvalue[ii - 1], ifieldlen[ii - 1]);
  }

  // 如果是增量抽取，绑定输入参数（已抽取数据的最大id）
  if(strlen(starg.incfield) != 0){
    stmt.bindin(1, &imaxincvalue);
  }

  if(stmt.execute() != 0){
    logfile.Write("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql, stmt.m_cda.message);
    return false;
  }

  PActive.UptATime();

  CFile File; // 用于操作xml文件

  while(true){
    memset(strfieldvalue, 0, sizeof(strfieldvalue));

    if(stmt.next() != 0) break;

    if(File.IsOpened() == false){
      crtxmlfilename(); // 生成xml文件名

      if(File.OpenForRename(strxmlfilename, "w+") == false){
        logfile.Write("File.OpenForRename(%s) failed.\n", strxmlfilename);
        return false;
      }
      File.Fprintf("<data>\n");
    }

    for(int ii = 1; ii <= ifieldcount; ++ii){
      File.Fprintf("<%s>%s</%s>", strfieldname[ii - 1], strfieldvalue[ii - 1], strfieldname[ii - 1]);
    }
    File.Fprintf("<endl/>\n");

    if((starg.maxcount > 0) && (stmt.m_cda.rpc % starg.maxcount == 0)){
      File.Fprintf("</data>\n");
      if(File.CloseAndRename() == false){
        logfile.Write("File.CloseAndRename() failed.\n");
        return false;
      }

      logfile.Write("生成文件%s(%d)。\n", strxmlfilename, starg.maxcount);
      PActive.UptATime();
    }
    // 更新自增字段的最大值
    if((strlen(starg.incfield) != 0) && (imaxincvalue < atol(strfieldvalue[incfieldpos]))) imaxincvalue = atol(strfieldvalue[incfieldpos]);

  }

  if(File.IsOpened() == true){
    File.Fprintf("</data>\n");
    if(File.CloseAndRename() == false){
      logfile.Write("File.CloseAndRename() failed.\n");
      return false;
    }
    if(starg.maxcount == 0)
      logfile.Write("生成文件%s(%d)。\n", strxmlfilename, stmt.m_cda.rpc);
    else
      logfile.Write("生成文件%s(%d)。\n", strxmlfilename, stmt.m_cda.rpc % starg.maxcount);
  }

  // 把最大的自增字段的值写入starg.incfilename文件中
  if(stmt.m_cda.rpc > 0) writeincfield();

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

// 生成xml文件名
void crtxmlfilename(){
  // xml全路径文件名=starg.outpath+starg.bfilename+当前时间+starg.efilename+序号.xml
  char strLocalTime[21];
  memset(strLocalTime, 0, sizeof(strLocalTime));
  LocalTime(strLocalTime, "yyyymmddhh24miss");

  static int iseq = 1;

  SNPRINTF(strxmlfilename, 300, sizeof(strxmlfilename), "%s/%s_%s_%s_%d.xml", starg.outpath, starg.bfilename, strLocalTime, starg.efilename, iseq++);

}

// 从starg.incfilename文件中获取已抽取数据的最大id
bool readincfield(){
  imaxincvalue = 0; // 自增字段最大值

  // 如果starg.incfield参数为空，表示不是增量抽取
  if(strlen(starg.incfield) == 0) return true;

  if(strlen(starg.connstr1) != 0){
    // 从数据库表中加载自增字段的最大值
    sqlstatement stmt(&conn1);
    stmt.prepare("select maxincvalue from T_MAXINCVALUE where pname=:1");
    stmt.bindin(1, starg.pname, 50);
    stmt.bindout(1, &imaxincvalue);
    stmt.execute();
    stmt.next();
  }else{
    // 从文件中加载自增字段的最大值
    CFile File;

    if(File.Open(starg.incfilename, "r") == false) return true;

    char strtemp[31];
    File.FFGETS(strtemp, 30);

    imaxincvalue = atol(strtemp);

    logfile.Write("上次已抽取数据的位置(%s=%ld)。\n", starg.incfield, imaxincvalue);
  }
 

  return true;
}

// 把已抽取数据的最大id写入starg.incfilename文件
bool writeincfield(){
  // 如果starg.incfield参数为空，表示不是增量抽取
  if(strlen(starg.incfield) == 0) return true;
  if(strlen(starg.connstr1) != 0){
    // 把自增字段的最大值写入数据库的表。
    sqlstatement stmt(&conn1);
    stmt.prepare("update from T_MAXINCVALUE set maxincvalue=:1 where pname=:2");

    if(stmt.m_cda.rc == 1064){
      // 如果表不存在，就创建表。
      conn1.execute("create table T_MAXINCVALUE(pname varchar(50),maxincvalue numeric(15),primary key(pname))");
      conn1.execute("insert into T_MAXINCVALUE values('%s',%ld)",starg.pname,imaxincvalue);
      conn1.commit();
      return true;
    }
    stmt.bindin(1, &imaxincvalue);
    stmt.bindin(2, starg.pname, 50);
    if(stmt.execute() != 0){
      logfile.Write("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql, stmt.m_cda.message);
      return false;
    }
    conn1.commit();
  }else{
    CFile File;

    if(File.Open(starg.incfilename, "w+") == false) {
      logfile.Write("File.Open(%s) failed.\n", starg.incfilename);
      return false;
    }

    // 把已抽取数据最大的id写入文件
    File.Fprintf("%ld", imaxincvalue);
    File.Close();
  }

  return true;
}