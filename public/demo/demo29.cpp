/*
 *  程序名：demo29.cpp，此程序演示开发框架中的CTimer类（计时器）的用法。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  CTimer Timer;

  printf("elapsed=%lf\n",Timer.Elapsed());
  sleep(1);
  printf("elapsed=%lf\n",Timer.Elapsed());
  sleep(1);
  printf("elapsed=%lf\n",Timer.Elapsed());
  usleep(1000);
  printf("elapsed=%lf\n",Timer.Elapsed());
  usleep(100);
  printf("elapsed=%lf\n",Timer.Elapsed());
  sleep(10);
  printf("elapsed=%lf\n",Timer.Elapsed());
}

