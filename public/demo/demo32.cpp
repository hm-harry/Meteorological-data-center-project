/*
 *  程序名：demo32.cpp，此程序演示开发框架中采用CDir类获取某目录及其子目录中的文件列表信息。
 *  作者：吴从周
*/
#include "../_public.h"

int main(int argc,char *argv[])
{
  if (argc != 2) { printf("请指定目录名。\n"); return -1; }

  CDir Dir;

  if (Dir.OpenDir(argv[1],"*.h,*cpp",100,true,true)==false)
  { 
    printf("Dir.OpenDir(%s) failed.\n",argv[1]); return -1; 
  }

  while(Dir.ReadDir()==true)
  {
    printf("filename=%s,mtime=%s,size=%d\n",Dir.m_FullFileName,Dir.m_ModifyTime,Dir.m_FileSize);
  }
}

