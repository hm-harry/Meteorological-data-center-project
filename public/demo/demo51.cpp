/*
 *  程序名：demo51.cpp，此程序演示采用开发框架的Cftp类上传文件。
 *  作者：吴从周
*/
#include "../_ftp.h"

int main(int argc,char *argv[])
{
  Cftp ftp;

  // 登录远程FTP服务器，请改为你自己服务器的ip地址。
  if (ftp.login("172.16.0.15:21","wucz","freecpluspwd",FTPLIB_PASSIVE) == false)
  {
    printf("ftp.login(172.16.0.15:21(wucz/freecpluspwd)) failed.\n"); return -1;
  }

  // 在ftp服务器上创建/home/wucz/tmp，注意，如果目录已存在，会返回失败。
  if (ftp.mkdir("/home/wucz/tmp")==false) { printf("ftp.mkdir() failed.\n"); return -1; }
  
  // 把ftp服务器上的工作目录切换到/home/wucz/tmp
  if (ftp.chdir("/home/wucz/tmp")==false) { printf("ftp.chdir() failed.\n"); return -1; }

  // 把本地的demo51.cpp上传到ftp服务器的当前工作目录。
  ftp.put("demo51.cpp","demo51.cpp");

  // 如果不调用chdir切换工作目录，以下代码也可以直接上传文件。
  // ftp.put("demo51.cpp","/home/wucz/tmp/demo51.cpp");
  
  printf("put demo51.cpp ok.\n");  
}

