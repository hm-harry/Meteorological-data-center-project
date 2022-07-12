/*
    程序名：crtsurfdatal.cpp 本程序用于生成全国气象站点观测的分钟数据
    作者：吴惠明
*/

#include "_public.h"

CLogFile logfile(10);

int main(int argc, char* argv[]){
  // inifile outpath logfile
  if(argc != 4){
    printf("Using:./crtsurfdatal inifile outpath logfile\n");
    printf("Example:/project/idc1/bin/crtsurfdata1 /project/idc1/ini/stcode.ini /tmp/sufdata /log/idc/crtsurfdata1.log\n\n");
    printf("inifile 全国气象站点参数文件名。\n");
    printf("outpath 全国气象站点数据文件存放的目录。\n");
    printf("logfile 本程序运行的日志文件名。\n\n");

    return -1;
  }

  if(logfile.Open(argv[3]) == false){
    printf("logfile.Open(%s) filed.\n", argv[3]);
    return -1;
  }
  logfile.Write("crtsurfdata1 开始运行。\n");

  //这里插入业务代码
  for(int ii = 0; ii < 1000; ++ii){
    logfile.Write("这是第%010d条日志记录。\n", ii);
  }

  logfile.Write("crtsurfdata1 运行结束。\n"); 
  return 0;
}
