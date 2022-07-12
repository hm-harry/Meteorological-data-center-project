/*
 *  程序名：demo45.cpp，此程序演示采用开发框架的CIniFile类加载参数文件。
 *  作者：吴从周
*/
#include "../_public.h"

// 用于存放本程序运行参数的数据结构。
struct st_args
{
  char logpath[301];
  char connstr[101];
  char datapath[301];
  char serverip[51];
  int  port;
  bool online;
}stargs;

int main(int argc,char *argv[])
{
  // 如果执行程序时输入的参数不正确，给出帮助信息。
  if (argc != 2) 
  { 
    printf("\nusing:/project/public/demo/demo45 inifile\n"); 
    printf("samples:/project/public/demo/demo45 /project/public/ini/hssms.xml\n\n"); 
    return -1;
  }

  // 加载参数文件。
  CIniFile IniFile;
  if (IniFile.LoadFile(argv[1])==false)
  {
    printf("IniFile.LoadFile(%s) failed.\n",argv[1]); return -1;
  } 

  // 获取参数，存放在stargs结构中。
  memset(&stargs,0,sizeof(struct st_args));
  IniFile.GetValue("logpath",stargs.logpath,300);
  IniFile.GetValue("connstr",stargs.connstr,100);
  IniFile.GetValue("datapath",stargs.datapath,300);
  IniFile.GetValue("serverip",stargs.serverip,50);
  IniFile.GetValue("port",&stargs.port);
  IniFile.GetValue("online",&stargs.online);
  
  printf("logpath=%s\n",stargs.logpath);
  printf("connstr=%s\n",stargs.connstr);
  printf("datapath=%s\n",stargs.datapath);
  printf("serverip=%s\n",stargs.serverip);
  printf("port=%d\n",stargs.port);
  printf("online=%d\n",stargs.online);

  // 以下可以写更多的主程序的代码。
}

