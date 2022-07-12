/*
 *  程序名：demo42.cpp，此程序演示采用开发框架的CLogFile类记录程序的运行日志。
 *  本程序修改demo40.cpp把输出的printf语句改为写日志文件。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  CLogFile logfile;

  // 打开日志文件，如果"/tmp/log"不存在，就创建它，但是要确保当前用户具备创建目录的权限。
  if (logfile.Open("/tmp/log/demo42.log")==false)
  { printf("logfile.Open(/tmp/log/demo42.log) failed.\n"); return -1; }

  logfile.Write("demo42程序开始运行。\n");

  CDir Dir;

  // 扫描/tmp/data目录下文件名匹配"surfdata_*.xml"的文件。
  if (Dir.OpenDir("/tmp/data","surfdata_*.xml")==false)
  { logfile.Write("Dir.OpenDir(/tmp/data) failed.\n"); return -1; }

  CFile File;

  while (Dir.ReadDir()==true)
  {
    logfile.Write("处理文件%s...",Dir.m_FullFileName);

    if (File.Open(Dir.m_FullFileName,"r")==false)
    { logfile.WriteEx("failed.File.Open(%s) failed.\n",Dir.m_FullFileName); return -1; }

    char strBuffer[301];

    while (true)
    {
      memset(strBuffer,0,sizeof(strBuffer));
      if (File.FFGETS(strBuffer,300,"<endl/>")==false) break; // 行内容以"<endl/>"结束。

      // logfile.Write("strBuffer=%s",strBuffer);

      // 这里可以插入解析xml字符串并把数据写入数据库的代码。
    }


    // 处理完文件中的数据后，关闭文件指针，并删除文件。
    File.CloseAndRemove();

    logfile.WriteEx("ok\n");
  }

  logfile.Write("demo42程序运行结束。\n");
}

