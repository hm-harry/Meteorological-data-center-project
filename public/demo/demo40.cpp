/*
 *  程序名：demo40.cpp，此程序演示开发框架中采用CDir类和CFile类处理数据文件的用法。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  CDir Dir;

  // 扫描/tmp/data目录下文件名匹配"surfdata_*.xml"的文件。
  if (Dir.OpenDir("/tmp/data","surfdata_*.xml")==false)
  {
    printf("Dir.OpenDir(/tmp/data) failed.\n"); return -1;
  }

  CFile File;

  while (Dir.ReadDir()==true)
  {
    printf("处理文件%s...",Dir.m_FullFileName);

    if (File.Open(Dir.m_FullFileName,"r")==false)
    {
      printf("failed.File.Open(%s) failed.\n",Dir.m_FullFileName); return -1;
    }

    char strBuffer[301];

    while (true)
    {
      memset(strBuffer,0,sizeof(strBuffer));
      if (File.FFGETS(strBuffer,300,"<endl/>")==false) break; // 行内容以"<endl/>"结束。

      // printf("strBuffer=%s",strBuffer);

      // 这里可以插入解析xml字符串并把数据写入数据库的代码。
    }

    // 处理完文件中的数据后，关闭文件指针，并删除文件。
    File.CloseAndRemove();

    printf("ok\n");
  }
}

