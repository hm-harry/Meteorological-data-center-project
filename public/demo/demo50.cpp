/*
 *  程序名：demo50.cpp，此程序演示采用开发框架的Cftp类获取服务器文件列表、时间和大小。
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

  // 获取服务器上/home/wucz/*.h文件列表，保存在本地的/tmp/list/tmp.list文件中。
  // 如果/tmp/list目录不存在，就创建它。
  if (ftp.nlist("/home/wucz/*.h","/tmp/list/tmp.list")==false) 
  { printf("ftp.nlist() failed.\n"); return -1; }

  CFile File;    // 采用开发框架的CFile类来操作list文件。
  char strFileName[301];

  File.Open("/tmp/list/tmp.list","r");  // 打开list文件。

  while(true)    // 获取每个文件的时间和大小。
  {
    if (File.Fgets(strFileName,300,true)==false) break;

    ftp.mtime(strFileName); // 获取文件时间。
    ftp.size(strFileName);  // 获取文件大小。
  
    printf("filename=%s,mtime=%s,size=%d\n",strFileName,ftp.m_mtime,ftp.m_size);   
  }

}

