// 本程序演示线程资源的回收（分离线程）。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void *thmain(void *arg);    // 线程主函数。

int main(int argc,char *argv[])
{
  pthread_t thid;

  pthread_attr_t attr;         // 申明线程属性的数据结构。
  pthread_attr_init(&attr);    // 初始化。
  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);  // 设置线程的属性。
  // 创建线程。
  if (pthread_create(&thid,&attr,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
  pthread_attr_destroy(&attr); // 销毁数据结构。

  // pthread_detach(pthread_self());

  sleep(5);

  int result=0;
  void *ret;
  printf("join...\n");
  result=pthread_join(thid,&ret);   printf("thid result=%d,ret=%ld\n",result,ret);
  printf("join ok.\n");
}

void *thmain(void *arg)    // 线程主函数。
{
  // pthread_detach(pthread_self());

  for (int ii=0;ii<3;ii++)
  {
    sleep(1); printf("pthmain sleep(%d) ok.\n",ii+1);
  }
  return (void *) 1;
}
