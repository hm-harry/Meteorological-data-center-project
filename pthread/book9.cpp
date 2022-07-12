// 本程序演示线程的取消。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int var=0;

void *thmain(void *arg);    // 线程主函数。

int main(int argc,char *argv[])
{
  pthread_t thid;

  // 创建线程。
  if (pthread_create(&thid,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }

  usleep(100); pthread_cancel(thid);

  int result=0;
  void *ret;
  printf("join...\n");
  result=pthread_join(thid,&ret);   printf("thid result=%d,ret=%ld\n",result,ret);
  printf("join ok.\n");

  printf("var=%d\n",var);
}

void *thmain(void *arg)    // 线程主函数。
{
  // pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

  for (var=0;var<400000000;var++)
  {
    ;
    pthread_testcancel();
  }
  return (void *) 1;
}
