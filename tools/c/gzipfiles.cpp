#include "_public.h"

// 程序退出和信号2、15的处理函数。
void EXIT(int sig);

int main(int argc,char *argv[])
{
  // 程序的帮助。
  if (argc != 4)
  {
    printf("\n");
    printf("Using:/project/tools1/bin/gzipfiles pathname matchstr timeout\n\n");

    printf("Example:/project/tools1/bin/gzipfiles /log/idc \"*.log.20*\" 0.02\n");
    printf("        /project/tools1/bin/gzipfiles /tmp/idc/surfdata \"*.xml,*.json\" 0.01\n");
    printf("        /project/tools1/bin/procctl 300 /project/tools1/bin/gzipfiles /log/idc \"*.log.20*\" 0.02\n");
    printf("        /project/tools1/bin/procctl 300 /project/tools1/bin/gzipfiles /tmp/idc/surfdata \"*.xml,*.json\" 0.01\n\n");

    printf("这是一个工具程序，用于压缩历史的数据文件或日志文件。\n");
    printf("本程序把pathname目录及子目录中timeout天之前的匹配matchstr文件全部压缩，timeout可以是小数。\n");
    printf("本程序不写日志文件，也不会在控制台输出任何信息。\n");
    printf("本程序调用/usr/bin/gzip命令压缩文件。\n\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程。
  // 但请不要用 "kill -9 +进程号" 强行终止。
  CloseIOAndSignal(true); signal(SIGINT,EXIT);  signal(SIGTERM,EXIT);

  // 获取文件超时的时间点。
  char strTimeOut[21];
  LocalTime(strTimeOut,"yyyy-mm-dd hh24:mi:ss",0-(int)(atof(argv[3])*24*60*60));

  CDir Dir;
  // 打开目录，CDir.OpenDir()
  if (Dir.OpenDir(argv[1],argv[2],10000,true)==false)
  {
    printf("Dir.OpenDir(%s) failed.\n",argv[1]); return -1;
  }

  char strCmd[1024]; // 存放gzip压缩文件的命令。

  // 遍历目录中的文件名。
  while (true)
  {
    // 得到一个文件的信息，CDir.ReadDir()
    if (Dir.ReadDir()==false) break;
  
    // 与超时的时间点比较，如果更早，就需要压缩
    if ( (strcmp(Dir.m_ModifyTime,strTimeOut)<0) && (MatchStr(Dir.m_FileName,"*.gz")==false) )
    {
      // 压缩文件，调用操作系统的gzip命令。
      SNPRINTF(strCmd,sizeof(strCmd),1000,"/usr/bin/gzip -f %s 1>/dev/null 2>/dev/null",Dir.m_FullFileName);
      if (system(strCmd)==0) 
        printf("gzip %s ok.\n",Dir.m_FullFileName);
      else
        printf("gzip %s failed.\n",Dir.m_FullFileName);
    }
  }

  return 0;
}

void EXIT(int sig)
{
  printf("程序退出，sig=%d\n\n",sig);

  exit(0);
}
