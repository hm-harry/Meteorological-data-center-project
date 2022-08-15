/*
  程序名：xmltodb.cpp，本程序是数据中心的公共功能模块，用于把xml文件入库到MySQL的表中
  作者：吴惠明
*/
#include"_public.h"
#include"_mysql.h"

// 表的列（字段）信息的结构体
struct st_columns{
  char colname[31];     // 列名
  char datatype[31];    // 列的数据类型，分为number，data和char三大类
  int collen;           // 列的长度，number固定20，data固定19，char的长度由表的结构决定
  int pkseq;            // 如果列是主键字段，存放主键字段的顺序，从1开始，不是主键为0
};

// 获取表全部的列和主键列信息的类
class CTABCOLS{
public:
  CTABCOLS();
  int m_allcount;  // 全部字段的个数
  int m_pkcount;   // 主键字段的个数

  vector<struct st_columns> m_vallcols;  // 存放全部字段信息的容器
  vector<struct st_columns> m_vpkcols;  // 存放主键字段信息的容器

  char m_allcols[3001];  // 全部字段，以字符串存放，中间用半角的逗号分割
  char m_pkcols[301];    // 全部的主键字段，以字符串存放，中间用半角的逗号分割

  void initdata();  // 成员变量初始化

  // 获取指定表的全部字段信息
  bool allcols(connection* conn, char* tablename);

  // 获取指定表的主键字段信息
  bool pkcols(connection* conn, char* tablename);
};

struct st_arg{
  char connstr[101];        // 数据库连接参数
  char charset[51];         // 数据库字符集
  char inifilename[301];    // 数据入库的参数配置文件
  char xmlpath[301];        // 待入库xml文件存放目录
  char xmlpathbak[301];     // xml文件入库后的备份目录
  char xmlpatherr[301];     // 入库失败的xml文件存放目录
  int timetvl;              // 本程序运行的时间间隔，本程序常驻内存
  int timeout;              // 进程心跳的超时时间
  char pname[51];           // 进程名，建议使用"dminingmysql_后缀"的方式
}starg;

// 显示程序的帮助
void _help();

// 把xml解析到参数starg结构体中
bool _xmltoarg(char *strxmlbuff);

CLogFile logfile;

CPActive PActive; // 进程心跳

connection conn;

CTABCOLS TABCOLS; // 获取表全部的字段和主键字段

// 业务处理主函数
bool  _xmltodb();

struct st_xmltotable{
  char filename[101];     // xml文件的匹配规则，用逗号分割
  char tname[31];         // 待入库的表名
  int uptbz;              // 更新标志：1-更新，2-不更新
  char execsql[301];      // 处理xml文件之间，执行的SQL语句
}stxmltotable;

vector<struct st_xmltotable> vxmltotable;  // 数据入库参数的容器

// 把数据入库的参数配置文件starg.inifilename加载到vxmltotable容器中
bool loadxmltotable();

// 从vxmltotable容器中查找xmlfilename的入库参数，存放在stxmltotable结构体中。
bool findxmltotable(char* xmlfilename);

// 把xml文件移动到备份目录或者错误目录
bool xmltobakerr(char* fullfilename, char* srcpath, char* dstpath);

// 程序退出和信号2/5的处理函数
void EXIT(int sig);

// 调用处理文件的子函数，返回值：0-成功，其他都是失败
int _xmltodb(char* fullfilename, char* filename);

char strinsertsql[10241]; // 插入表的SQL语句
char strupdatesql[10241]; // 更新表的SQL语句

// 拼接生成插入和更新表数据的SQL
void crtsql();

// prepare插入和更新的sql语句，绑定输入变量
#define MAXCOLCOUNT 300 // 每个表字段的最大值
#define MAXCOLLEN 100 // 表字段值的最大长度
char strcolvalue[MAXCOLCOUNT][MAXCOLLEN + 1]; // 存放从xml每一行中解析出来的值
sqlstatement stmtins, stmtupt;
void preparesql();
 
int main(int argc, char* argv[]){
  // 把服务器上某个目录的文件全部上传到本地目录（可以指定文件名匹配规则）。
  if(argc != 3){  _help();  return -1;}

  // 关闭全部的信号和输入输出
  // 设置信号，在shell状态下可用"kill + 进程号"正常终止进程
  // 但请不要用"kill -9 + 进程号"强行终止
  // CloseIOAndSignal();
  signal(SIGINT, EXIT);
  signal(SIGTERM, EXIT);

  // 打开日志文件
  if(logfile.Open(argv[1], "a+") == false){ printf("打开日志文件失败（%s）\n", argv[1]);  return -1;}

  // 解析xml，得到程序运行的参数
  if(_xmltoarg(argv[2]) == false) return -1;

  if(conn.connecttodb(starg.connstr, starg.charset) != 0){
    logfile.Write("connect database(%s) failed.\n%s\n", starg.connstr, conn.m_cda.message);
    return -1;
  }
  logfile.Write("connect database(%s) ok.\n", starg.connstr);

  // 业务处理主函数
  _xmltodb();

  return 0;
}

void EXIT(int sig){
  printf("进程退出，sig = %d\n\n", sig);

  exit(0);
}

void _help(){
    printf("\n");
    printf("Using:/project/tools1/bin/xmltodb logfilename xmlbuffer\n\n");

    printf("Sample:/project/tools1/bin/procctl 3600 /project/tools1/bin/xmltodb /log/idc/xmltodb_vip1.log \
    \"<connstr>127.0.0.1,root,whmhhh1998818,mysql,3306</connstr><charset>utf8</charset><inifilename>/project/tools1/ini/xmltodb.xml</inifilename>\
    <xmlpath>/idcdata/xmltodb/vip1</xmlpath><xmlpathbak>/idcdata/xmltodb/vip1bak</xmlpathbak><xmlpatherr>/idcdata/xmltodb/vip1err</xmlpatherr>\
    <timetvl>5</timetvl><timeout>50</timeout><pname>xmltodb_vip1</pname>\"\n\n");

    printf("本程序是数据中心的公共功能模块，用于把xml文件入库到MySQL的表中\n");
    printf("logfilename     是本程序的运行的日志文件。\n");
    printf("xmlbuffer       本程序运行的参数，用xml表示，具体如下：\n");

    printf("connstr         数据库连接参数,格式：ip,username,password,dbname,port。\n");
    printf("charset         数据库的字符集，这个参数要与数据源数据库保持一致，否则会出现中文乱码的现象\n");
    printf("inifilename     数据入库的参数配置文件\n");
    printf("xmlpath         待入库xml文件存放目录。\n");
    printf("xmlpathbak      xml文件入库后的备份目录\n");
    printf("xmlpatherr      入库失败的xml文件存放目录。\n");
    printf("timetvl         本程序运行的时间间隔，单位秒，视业务需求而定，2-30之间。\n");
    printf("timeout         本程序的超时时间，单位：秒，视业务需求而定，建议设置30以上。\n");
    printf("pname           进程名，尽可能采用易懂的、与其他进程不同的名称，方便故障排查。\n\n\n");

}

bool _xmltoarg(char *strxmlbuff){
  memset(&starg, 0, sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuff, "connstr", starg.connstr, 100);
  if(strlen(starg.connstr) == 0){ logfile.Write("connstr is null.\n");  return false;}

  GetXMLBuffer(strxmlbuff, "charset", starg.charset, 50);
  if(strlen(starg.charset) == 0){ logfile.Write("charset is null.\n");  return false;}

  GetXMLBuffer(strxmlbuff, "inifilename", starg.inifilename, 1000);
  if(strlen(starg.inifilename) == 0){ logfile.Write("inifilename is null.\n");  return false;}

  GetXMLBuffer(strxmlbuff, "xmlpath", starg.xmlpath, 500);
  if(strlen(starg.xmlpath) == 0){ logfile.Write("xmlpath is null.\n");  return false;}

  GetXMLBuffer(strxmlbuff, "xmlpathbak", starg.xmlpathbak, 500);
  if(strlen(starg.xmlpathbak) == 0){  logfile.Write("xmlpathbak is null.\n"); return false;}

  GetXMLBuffer(strxmlbuff, "xmlpatherr", starg.xmlpatherr, 30);
  if(strlen(starg.xmlpatherr) == 0){  logfile.Write("xmlpatherr is null.\n"); return false;}

  GetXMLBuffer(strxmlbuff, "timetvl", &starg.timetvl);
  if(starg.timetvl < 2) starg.timetvl = 2;
  if(starg.timetvl > 30) starg.timetvl = 30;

  GetXMLBuffer(strxmlbuff, "timeout", &starg.timeout);
  if(starg.timeout == 0){ logfile.Write("timeout is null.\n");  return false;}

  GetXMLBuffer(strxmlbuff, "pname", starg.pname, 50);
  if(strlen(starg.pname) == 0){ logfile.Write("pname is null.\n");  return false;}

  return true;
}

// 业务处理主函数
bool  _xmltodb(){
  int counter = 50;  // 加载入库参数的计数器，初始化为50是为了第一次进入循环就加载参数

  CDir Dir;

  while(true){
    if(counter++ > 30){
      counter = 0; // 重新计数
      // 把数据入库的参数配置文件starg.inifilename加载到容器中
      if(loadxmltotable() == false) return false;
    }

    // 打开starg.xmlpath目录
    if(Dir.OpenDir(starg.xmlpath, "*.XML", 10000, false, true) == false){
      logfile.Write("Dir.OpenDir(%s) failed.\n", starg.xmlpath);
      return false;
    }

    while(true){
      // 读取目录得到一个xml文件
      if(Dir.ReadDir() == false) break;

      logfile.Write("处理文件%s...", Dir.m_FullFileName);

      // 处理xml文件
      // 调用处理文件的子函数
      int iret = _xmltodb(Dir.m_FullFileName, Dir.m_FileName);

      // 处理xml文件成功，写日志，备份文件
      if(iret == 0){
        logfile.WriteEx("ok.\n");
        // 把xml文件移动到starg.xmlpathbak参数指定的目录中
        if(xmltobakerr(Dir.m_FullFileName, starg.xmlpath, starg.xmlpathbak) == false) return false;
      }

      // 如果处理xml文件失败，分多钟情况
      if((iret == 1) || (iret == 2)){// iret==1，找不到入库参数 2,待入库的表不存在
        if(iret == 1) logfile.WriteEx("failed，没有配置入库参数。\n");
        if(iret == 2) logfile.WriteEx("failed，待入库的表（%s）不存在。\n", stxmltotable.tname);
        // 把xml文件移动到starg.xmlpatherr参数指定的目录中
        if(xmltobakerr(Dir.m_FullFileName, starg.xmlpath, starg.xmlpatherr) == false) return false;
      }

      // 数据库错误，函数返回，程序退出
      if(iret == 4){
        logfile.WriteEx("failed,数据库错误。\n");
        return false;
      }
    }
    break;
    sleep(starg.timetvl);
  }
  return true;
}

// 把数据入库的参数配置文件starg.inifilename加载到vxmltotable容器中
bool loadxmltotable(){
  vxmltotable.clear();

  CFile File;
  if(File.Open(starg.inifilename, "r") == false){
    logfile.Write("File.Open(%s) failed.\n", starg.inifilename);
    return false;
  }

  char strBuffer[501];
  while(true){
    if(File.FFGETS(strBuffer, 500, "<endl/>") == false) break;

    memset(&stxmltotable, 0, sizeof(struct st_xmltotable));

    GetXMLBuffer(strBuffer, "filename", stxmltotable.filename, 100);
    GetXMLBuffer(strBuffer, "tname", stxmltotable.tname, 30);
    GetXMLBuffer(strBuffer, "uptbz", &stxmltotable.uptbz);
    GetXMLBuffer(strBuffer, "execsql", stxmltotable.execsql, 300);

    vxmltotable.push_back(stxmltotable);
  }
  logfile.Write("loadxmltotable(%s) ok.\n", starg.inifilename);

  return true;
}

// 从vxmltotable容器中查找xmlfilename的入库参数，存放在stxmltotable结构体中。
bool findxmltotable(char* xmlfilename){
  for(int ii = 0; ii < vxmltotable.size(); ++ii){
    if(MatchStr(xmlfilename, vxmltotable[ii].filename) == true){
      memcpy(&stxmltotable, &vxmltotable[ii], sizeof(struct st_xmltotable));
      return true;
    }
  }
  return false;
}

// 调用处理文件的子函数，返回值：0-成功，其他都是失败
int _xmltodb(char* fullfilename, char* filename){
  // 从vxmltotable容器中查找filename的入库参数，存放在stxmltotable结构体中
  if(findxmltotable(filename) == false) return 1;

  // 获取表的全部字段和主键信息
  // 在本程序运行过程中，如果数据库出现异常，一定会在这里发现，错误信息返回4
  if(TABCOLS.allcols(&conn, stxmltotable.tname) == false) return 4;
  if(TABCOLS.pkcols(&conn, stxmltotable.tname) == false) return 4;

  // 如果TABCOLS.m_allcount为0，说明表根本不存在，返回2
  if(TABCOLS.m_allcount == 0) return 2;

  // 拼接生成插入和更新表数据的SQL
  crtsql();

  // prepare插入和更新的sql语句，绑定输入变量
  preparesql();

  // 在处理xml文件之前，如果stxmltotable.execsql不为空，就执行它

  // 打开xml文件
  // while(true){
  //   // 从xml文件中读取一行

  //   // 解析xml，存放在已绑定的输入变量中

  //   // 执行插入和更新的sql
  // }

  return 0;
}

// 把xml文件移动到备份目录或者错误目录
bool xmltobakerr(char* fullfilename, char* srcpath, char* dstpath){
  char dstfilename[301];// 目标文件名
  STRCPY(dstfilename, sizeof(dstfilename), fullfilename);

  UpdateStr(dstfilename, srcpath, dstpath, false);

  if(RENAME(fullfilename, dstfilename) == false){
    logfile.Write("RENAME(%s, %s) failed.\n",fullfilename, dstfilename);
    return false;
  }
  return true;
}

CTABCOLS::CTABCOLS(){
  initdata();  // 调用成员变量初始化函数
}

void CTABCOLS::initdata(){  // 成员变量初始化
  m_allcount = m_pkcount = 0;
  m_vallcols.clear();
  m_vpkcols.clear();
  memset(m_allcols, 0, sizeof(m_allcols));
  memset(m_pkcols, 0, sizeof(m_pkcols));
}

// 获取指定表的全部字段信息
bool CTABCOLS::allcols(connection* conn, char* tablename){
  m_allcount = 0;
  m_vallcols.clear();
  memset(m_allcols, 0, sizeof(m_allcols));

  struct st_columns stcolumns;
  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select lower(column_name), lower(data_type), character_maximum_length from information_schema.COLUMNS where table_name = :1");
  
  stmt.bindin(1, tablename, 30);
  stmt.bindout(1, stcolumns.colname, 30);
  stmt.bindout(2, stcolumns.datatype, 30);
  stmt.bindout(3, &stcolumns.collen);

  if(stmt.execute() != 0) return false;

  while(true){
    memset(&stcolumns, 0, sizeof(struct st_columns));

    if(stmt.next() != 0) break;

    // 列的数据类型，分为number，data和char三大类
    if(strcmp(stcolumns.datatype, "char") == 0) strcpy(stcolumns.datatype, "char");
    if(strcmp(stcolumns.datatype, "varchar") == 0) strcpy(stcolumns.datatype, "char");
    if(strcmp(stcolumns.datatype, "datetime") == 0) strcpy(stcolumns.datatype, "date");
    if(strcmp(stcolumns.datatype, "timestamp") == 0) strcpy(stcolumns.datatype, "date");
    if(strcmp(stcolumns.datatype, "tingint") == 0) strcpy(stcolumns.datatype, "number");
    if(strcmp(stcolumns.datatype, "smallint") == 0) strcpy(stcolumns.datatype, "number");
    if(strcmp(stcolumns.datatype, "mediumint") == 0) strcpy(stcolumns.datatype, "number");
    if(strcmp(stcolumns.datatype, "int") == 0) strcpy(stcolumns.datatype, "number");
    if(strcmp(stcolumns.datatype, "integer") == 0) strcpy(stcolumns.datatype, "number");
    if(strcmp(stcolumns.datatype, "bigint") == 0) strcpy(stcolumns.datatype, "number");
    if(strcmp(stcolumns.datatype, "numeric") == 0) strcpy(stcolumns.datatype, "number");
    if(strcmp(stcolumns.datatype, "decimal") == 0) strcpy(stcolumns.datatype, "number");
    if(strcmp(stcolumns.datatype, "float") == 0) strcpy(stcolumns.datatype, "number");
    if(strcmp(stcolumns.datatype, "double") == 0) strcpy(stcolumns.datatype, "number");

    // 如果业务有需要，可以修改上面代码，增加对更多数据类型的支持
    // 如果字段的数据类型不在上面列出来的中，忽略它
    if(strcmp(stcolumns.datatype, "char") != 0 && strcmp(stcolumns.datatype, "date") != 0 && strcmp(stcolumns.datatype, "number") != 0){
      continue;
    }

    // 如果字段类型是date，把长度设置为19 yyyy-mm-dd hh:mi:ss
    if(strcmp(stcolumns.datatype, "date") == 0) stcolumns.collen = 19;

    // 如果字段类型是number，把长度设置为20
    if(strcmp(stcolumns.datatype, "number") == 0) stcolumns.collen = 20;

    strcat(m_allcols, stcolumns.colname); strcat(m_allcols, ",");

    m_vallcols.push_back(stcolumns);

    m_allcount++;
  }
  // 删除m_allcols最后一个多余的逗号
  if(m_allcount > 0) m_allcols[strlen(m_allcols) - 1] = 0;

  return true;
}

// 获取指定表的主键字段信息
bool CTABCOLS::pkcols(connection* conn, char* tablename){
  m_pkcount = 0;
  memset(m_pkcols, 0, sizeof(m_pkcols));
  m_vpkcols.clear();

  struct st_columns stcolumns;

  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select lower(column_name), seq_in_index from information_schema.STATISTICS where table_name = :1 and index_name = 'primary' order by seq_in_index");

  stmt.bindin(1, tablename, 30);
  stmt.bindout(1, stcolumns.colname, 30);
  stmt.bindout(2, &stcolumns.pkseq);

  if(stmt.execute() != 0) return false;

  while(true){
    memset(&stcolumns, 0, sizeof(struct st_columns));
    if(stmt.next() != 0) break;
    strcat(m_pkcols, stcolumns.colname); strcat(m_pkcols, ",");
    m_vpkcols.push_back(stcolumns);
    m_pkcount++;
  }
  if(m_pkcount > 0) m_pkcols[strlen(m_pkcols) - 1] == 0;  // 删除m_pkcols最后一个多余的逗号

  return true;
}

// 拼接生成插入和更新表数据的SQL
void crtsql(){
  memset(strinsertsql, 0, sizeof(strinsertsql)); // 插入表的SQL语句
  memset(strupdatesql, 0, sizeof(strinsertsql)); // 更新表的SQL语句

  // 生成插入表的SQL语句 insert into 表名(%s) values(%s)
  char strinsertp1[3001]; // insert语句的字段列表
  char strinsertp2[3001]; // insert语句values后面的内容

  memset(strinsertp1, 0, sizeof(strinsertp1));
  memset(strinsertp2, 0, sizeof(strinsertp2));

  int colseq = 1; // values部分字段的序号
  for(int ii = 0; ii < TABCOLS.m_vallcols.size(); ++ii){
    // upttime和keyid这两个字段不需要处理]
    if((strcmp(TABCOLS.m_vallcols[ii].colname, "upttime") == 0) ||(strcmp(TABCOLS.m_vallcols[ii].colname, "keyid") == 0)) continue;

    // 拼接strinsertp1
    strcat(strinsertp1, TABCOLS.m_vallcols[ii].colname); strcat(strinsertp1, ",");

    // 拼接strinsertp2
    char strtemp[101];
    if(strcmp(TABCOLS.m_vallcols[ii].datatype, "date") != 0){
      SNPRINTF(strtemp, 100, sizeof(strtemp), ":%d", colseq++);
    }else{
      SNPRINTF(strtemp, 100, sizeof(strtemp), "str_to_date(:%d, '%%%%Y%%%%m%%%%d%%%%H%%%%i%%%%s')", colseq++);
    }
    strcat(strinsertp2, strtemp); strcat(strinsertp2, ",");
  }
  strinsertp1[strlen(strinsertp1) - 1] = 0; // 把多余的逗号删除
  strinsertp2[strlen(strinsertp2) - 1] = 0; // 把多余的逗号删除

  SNPRINTF(strinsertsql, 10241, sizeof(strinsertsql), "insert into %s(%s) values(%s)", stxmltotable.tname, strinsertp1, strinsertp2);

  // logfile.Write("strinsertsql = %s\n", strinsertsql);

  if(stxmltotable.uptbz != 1) return;
  // 生成修改表的SQL语句

  // 更新TABCOLS.m_vallcols中的pkseq字段，在拼接update语句的时候要用它
  for(int ii = 0; ii < TABCOLS.m_vpkcols.size(); ++ii){
    for(int jj = 0; jj < TABCOLS.m_vallcols.size(); ++jj){
      if(strcmp(TABCOLS.m_vpkcols[ii].colname, TABCOLS.m_vallcols[jj].colname) == 0){
        // 更新m_vallcols容器中的pkseq
        TABCOLS.m_vallcols[jj].pkseq = TABCOLS.m_vpkcols[ii].pkseq;
        break;
      }
    }
  }
  // 先拼接update语句开始的部分
  sprintf(strupdatesql, "update %s set ", stxmltotable.tname);

  // 拼接update语句set后面的部分
  colseq = 1;
  for(int ii = 0; ii < TABCOLS.m_vallcols.size(); ++ii){
    // keyid字段不需要处理
    if(strcmp(TABCOLS.m_vallcols[ii].colname, "keyid") == 0) continue;

    // 如果是主键字段，也不需要拼接
    if(TABCOLS.m_vallcols[ii].pkseq != 0) continue;

    // upttime直接等于now()，为了考虑数据的兼容性
    if(strcmp(TABCOLS.m_vallcols[ii].colname, "upttime") == 0){
      strcat(strupdatesql, "upttime = now(), "); continue;
    }

    // 其他字段区分date字段和非date字段
    char strtemp[101];
    if(strcmp(TABCOLS.m_vallcols[ii].datatype, "date") != 0){
      SNPRINTF(strtemp, 100, sizeof(strtemp), "%s = :%d", TABCOLS.m_vallcols[ii].colname, colseq++);
    }else{
      SNPRINTF(strtemp, 100, sizeof(strtemp), "%s = str_to_date(:%d, '%%%%Y%%%%m%%%%d%%%%H%%%%i%%%%s')", TABCOLS.m_vallcols[ii].colname, colseq++);
    }
    strcat(strupdatesql, strtemp); strcat(strupdatesql, ",");
  }
  strupdatesql[strlen(strupdatesql) - 1] = 0; // 删除最后一个多余的逗号

  // 然后拼接update语句where后面的部分
  strcat(strupdatesql, " where 1=1 "); // 用1=1是为了后面拼接方便
  for(int ii = 0; ii < TABCOLS.m_vallcols.size(); ++ii){
    if(TABCOLS.m_vallcols[ii].pkseq == 0) continue; // 如果不是主键字段，跳过

    // 把主键字段拼接到update语句中，需要区分date字段和非date字段
    char strtemp[101];
    if(strcmp(TABCOLS.m_vallcols[ii].datatype, "date") != 0){
      SNPRINTF(strtemp, 100, sizeof(strtemp), " and %s = :%d", TABCOLS.m_vallcols[ii].colname, colseq++);
    }else{
      SNPRINTF(strtemp, 100, sizeof(strtemp), " and %s = str_to_date(:%d, '%%%%Y%%%%m%%%%d%%%%H%%%%i%%%%s')", TABCOLS.m_vallcols[ii].colname, colseq++);
    }
    strcat(strupdatesql, strtemp);
  }
  logfile.Write("strupdatesql = %s\n", strupdatesql);

}

// char strcolvalue[MAXCOLCOUNT][MAXCOLLEN + 1]; // 存放从xml每一行中解析出来的值
// sqlstatement stmtins, stmtupt;
// prepare插入和更新的sql语句，绑定输入变量
void preparesql(){
  // 绑定插入sql语句的输入变量
  stmtins.connect(&conn);
  stmtins.prepare(strinsertsql);
  int colseq = 1; // values部分字段的编号

  for(int ii = 0; ii < TABCOLS.m_vallcols.size(); ++ii){
    // upttime和keyid这两个字段不需要处理
    if((strcmp(TABCOLS.m_vallcols[ii].colname, "upttime") == 0) || (strcmp(TABCOLS.m_vallcols[ii].colname, "keyid") == 0)) continue;
    stmtins.bindin(colseq++, strcolvalue[ii], TABCOLS.m_vallcols[ii].collen);
  }

  // 绑定更新sql语句的输入变量
  // 如果入库参数中指定了表数据不需要更新，就不处理update语句了，函数返回
  if(stxmltotable.uptbz != 1) return;
  stmtupt.connect(&conn);
  stmtupt.prepare(strupdatesql);
  colseq = 1;
  
  // 绑定set部分输入参数
  for(int ii = 0; ii < TABCOLS.m_vallcols.size(); ++ii){
    // upttime和keyid这两个字段不需要处理
    if((strcmp(TABCOLS.m_vallcols[ii].colname, "upttime") == 0) || (strcmp(TABCOLS.m_vallcols[ii].colname, "keyid") == 0)) continue;

    // 如果是主键字段，不需要绑定
    if(TABCOLS.m_vallcols[ii].pkseq != 0) continue;

    stmtupt.bindin(colseq++, strcolvalue[ii], TABCOLS.m_vallcols[ii].collen);
  }

  // 绑定where部分输入参数
  for(int ii = 0; ii < TABCOLS.m_vallcols.size(); ++ii){
    // 如果是不是主键字段，不需要绑定
    if(TABCOLS.m_vallcols[ii].pkseq == 0) continue;

    stmtupt.bindin(colseq++, strcolvalue[ii], TABCOLS.m_vallcols[ii].collen);
  }
}