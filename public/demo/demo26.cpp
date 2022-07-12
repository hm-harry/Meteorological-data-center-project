/*
 *  程序名：demo26.cpp，此程序演示开发框架中整数表示的时间和字符串表示的时间之间的转换。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  time_t ltime;
  char strtime[20];

  memset(strtime,0,sizeof(strtime));
  strcpy(strtime,"2020-01-01 12:35:22");

  ltime=strtotime(strtime);    // 转换为整数的时间
  printf("ltime=%ld\n",ltime); // 输出ltime=1577853322
  
  memset(strtime,0,sizeof(strtime));
  timetostr(ltime,strtime,"yyyy-mm-dd hh24:mi:ss");  // 转换为字符串的时间
  printf("strtime=%s\n",strtime);     // 输出strtime=2020-01-01 12:35:22
}

