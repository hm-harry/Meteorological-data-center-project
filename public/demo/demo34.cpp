/*
 *  程序名：demo34.cpp，此程序演示开发框架的文件操作函数的用法
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  // 删除文件。
  if (REMOVE("/project/public/lib_public.a")==false)
  {
    printf("REMOVE(/project/public/lib_public.a) %d:%s\n",errno,strerror(errno));
  }

  // 重命名文件。
  if (RENAME("/project/public/lib_public.so","/tmp/aaa/bbb/ccc/lib_public.so")==false)
  {
    printf("RENAME(/project/public/lib_public.so) %d:%s\n",errno,strerror(errno));
  }

  // 复制文件。
  if (COPY("/project/public/libftp.a","/tmp/root/aaa/bbb/ccc/libftp.a")==false)
  {
    printf("COPY(/project/public/libftp.a) %d:%s\n",errno,strerror(errno));
  }

  // 获取文件的大小。
  printf("size=%d\n",FileSize("/project/public/_public.h"));

  // 重置文件的时间。
  UTime("/project/public/_public.h","2020-01-05 13:37:29");

  // 获取文件的时间。
  char mtime[21]; memset(mtime,0,sizeof(mtime));
  FileMTime("/project/public/_public.h",mtime,"yyyy-mm-dd hh24:mi:ss");
  printf("mtime=%s\n",mtime);   // 输出mtime=2020-01-05 13:37:29
}

