/*
 *  程序名：demo28.cpp，此程序演示开发框架中采用AddTime进行时间的运算。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  time_t ltime;
  char strtime[20];

  memset(strtime,0,sizeof(strtime));
  strcpy(strtime,"2020-01-01 12:35:22");

  AddTime(strtime,strtime,0-1*24*60*60); // 减一天。
  printf("strtime=%s\n",strtime);     // 输出strtime=2019-12-31 12:35:22
  
  AddTime(strtime,strtime,2*24*60*60); // 加两天。
  printf("strtime=%s\n",strtime);     // 输出strtime=2020-01-02 12:35:22
}

