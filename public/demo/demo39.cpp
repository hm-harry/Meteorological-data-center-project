/*
 *  程序名：demo39.cpp，此程序演示开发框架中采用CFile类生成数据文件的用法。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  CFile File;

  char strLocalTime[21];   // 用于存放系统当前的时间，格式yyyymmddhh24miss。
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss");  // 获取系统当前时间。
  
  // 生成绝对路径的文件名，目录/tmp/data，文件名：前缀(surfdata_)+时间+后缀(.xml)。
  char strFileName[301];
  SNPRINTF(strFileName,sizeof(strFileName),300,"/tmp/data/surfdata_%s.xml",strLocalTime);

  // 采用OpenForRename创建文件，实际创建的文件名例如/tmp/data/surfdata_20200101123000.xml.tmp。
  if (File.OpenForRename(strFileName,"w")==false)
  {
    printf("File.OpenForRename(%s) failed.\n",strFileName); return -1;
  }

  // 这里可以插入向文件写入数据的代码。
  // 写入文本数据用Fprintf方法，写入二进制数据用Fwrite方法。

  // 向文件中写入两行超女数据。
  File.Fprintf("<data>\n"\
     "<name>妲已</name><age>28</age><sc>火辣</sc><yz>漂亮</yz><memo>商要亡，关我什么事。</memo><endl/>\n"\
     "<name>西施</name><age>25</age><sc>火辣</sc><yz>漂亮</yz><memo>1、中国排名第一的美女；\n"\
     "2、男朋友是范蠡；\n"\
     "3、老公是夫差，被勾践弄死了。</memo><endl/>\n"\
     "</data>\n");

  //sleep(30);   // 停止30秒，用ls /tmp/data/*.tmp可以看到生成的临时文件。

  // 关闭文件指针，并把临时文件名改为正式的文件名。
  // 注意，不能用File.Close()，因为Close方法是关闭文件指针，并删除临时文件。
  // CFile类的析构函数调用的是Close方法。
  File.CloseAndRename();
}

