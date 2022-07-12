// 本程序演示线程和信号。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

void func(int sig)
{
  printf("func catch signal %d\n",sig);
}

void *thmain(void *arg);    // 线程主函数。

int main(int argc,char *argv[])
{
  signal(2,func);

  // 创建线程。
  pthread_t thid;
  if (pthread_create(&thid,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }

  sleep(5); pthread_kill(thid,15); sleep(100);

  printf("join ...\n");
  pthread_join(thid,NULL); 
  printf("join ok.\n");
}

void *thmain(void *arg)    // 线程主函数。
{
  printf("sleep ....\n");
  sleep(10);
  printf("sleep ok.\n");

  return (void *) 1;
}
