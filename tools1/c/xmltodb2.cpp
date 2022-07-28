/*
  程序名：xmltodb.cpp，本程序是数据中心的公共功能模块，用于把xml文件入库到MySQL的表中
  作者：吴惠明
*/
#include"_public.h"
#include"_mysql.h"

#define MAXFIELDCOUNT 100 // 结果集字段的最大数
int MAXFIELDLEN = -1;   // 结果集字段值的最大长度

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
      if(iret == 1){// iret==1，找不到入库参数
        logfile.WriteEx("failed，没有配置入库参数。\n");
        // 把xml文件移动到starg.xmlpatherr参数指定的目录中
        if(xmltobakerr(Dir.m_FullFileName, starg.xmlpath, starg.xmlpatherr) == false) return false;
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
  if(findxmltotable(filename) == false) return 1;
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