#include"_public.h"
#include"_ftp.h"

struct st_arg{
  char host[31]; // 远程服务器的IP和端口
  int mode; // 传输模式 1-被动模式 2-主动模式 缺省采用被动模式
  char username[31]; // 远程服务器ftp的用户名。
  char password[31]; // 远程服务器ftp的密码
  char localpath[301]; // 本地文件存放的目录。
  char remotepath[301]; // 远程服务器存放文件的目录。
  char matchname[101]; // 待下载文件匹配的规则。
}starg;

CLogFile logfile;

Cftp ftp;

// 程序退出和信号2/5的处理函数
void EXIT(int sig);

void _help();

// 把xml解析到参数starg结构体中
bool _xmltoarg(char *strxmlbuff);

int main(int argc, char* argv[]){
  // 第一步计划， 把服务器上某个目录的文件全部下载到本地目录（可以指定文件名匹配规则）。
  if(argc != 3){
    _help();
    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号，在shell状态下可用"kill + 进程号"正常终止进程
  // 但请不要用"kill -9 + 进程号"强行终止
  //CloseIOAndSignal();
  signal(SIGINT, EXIT);
  signal(SIGTERM, EXIT);

  // 打开日志文件
  if(logfile.Open(argv[1], "a+") == false){
    printf("打开日志文件失败（%s）\n", argv[1]);
    return -1;
  }

  // 解析xml，得到程序运行的参数
  if(_xmltoarg(argv[2]) == false) return -1;


  return 0;
}

void EXIT(int sig){
  printf("进程退出，sig = %d\n\n", sig);

  exit(0);
}

void _help(){
    printf("\n");
    printf("Using:/project/tools1/bin/ftpgetfiles logfilename xmlbuffer\n\n");

    printf("Sample:/project/tools1/bin/procctl 30 /project/tools1/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log \"<host>172.29.193.250</host><mode>1</mode><username>wuhm</username><password>whmhhh1998818</password><localpath>/idcdata/surfdata</localpath><remotepath>/tmp/idc/surfdata</remotepath><matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname>\"\n\n\n");

    printf("本程序是通用的功能模块，用于把远程ftp服务器的文件下载到本地目录。\n");
    printf("logfilename是本程序的运行的日志文件。\n");
    printf("xmlbuffer为文件下载的参数，如下：\n");
    printf("<host>172.29.193.250</host> 远程服务器的IP和端口。\n");
    printf("<mode>1</mode> 传输模式 1-被动模式 2-主动模式 缺省采用被动模式\n");
    printf("<username>wuhm</username> 远程服务器ftp的用户名。\n");
    printf("<password>whmhhh1998818</password> 远程服务器ftp的密码\n");
    printf("<localpath>/idcdata/surfdata</localpath> 本地文件存放的目录。\n");
    printf("<remotepath>/tmp/idc/surfdata</remotepath> 远程服务器存放文件的目录。\n");
    printf("<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname> 待下载文件匹配的规则。"\
                    "不匹配的文件不会被下载，本字段尽可能设置准确,不建议用*匹配全部的文件。\n\n\n");

}

bool _xmltoarg(char *strxmlbuff){
  memset(&starg, 0, sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuff, "host", starg.host, 30); // 远程服务器的IP和端口
  if(strlen(starg.host) == 0){
    logfile.Write("host is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "mode", &starg.mode);// 传输模式 1-被动模式 2-主动模式 缺省采用被动模式
  if(starg.mode != 2) starg.mode = 1;

  GetXMLBuffer(strxmlbuff, "username", starg.username, 30); // 远程服务器ftp的用户名。
  if(strlen(starg.username) == 0){
    logfile.Write("username is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "password", starg.password, 30); // 远程服务器ftp的密码
  if(strlen(starg.password) == 0){
    logfile.Write("password is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "remotepath", starg.remotepath, 300); // 远程服务器存放文件的目录。
  if(strlen(starg.remotepath) == 0){
    logfile.Write("remotepath is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "localpath", starg.localpath, 300); // 本地文件存放的目录。
  if(strlen(starg.localpath) == 0){
    logfile.Write("localpath is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuff, "matchname", starg.matchname, 30); // 待下载文件匹配的规则。
  if(strlen(starg.matchname) == 0){
    logfile.Write("matchname is null.\n");
    return false;
  }
  return true;
}
