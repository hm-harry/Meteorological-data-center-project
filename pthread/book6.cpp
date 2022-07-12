// 本程序演示线程资源的回收，用pthread_join非分离的线程。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void *thmain1(void *arg);    // 线程主函数。
void *thmain2(void *arg);    // 线程主函数。

int main(int argc,char *argv[])
{
  pthread_t thid1,thid2;

  // 创建线程。
  if (pthread_create(&thid1,NULL,thmain1,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
  if (pthread_create(&thid2,NULL,thmain2,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
sleep(10);
  void *ret;
  printf("join...\n");
  int result=0;
  result=pthread_join(thid2,&ret);   printf("thid2 result=%d,ret=%ld\n",result,ret);
  result=pthread_join(thid1,&ret);   printf("thid1 result=%d,ret=%ld\n",result,ret);
  ret=0;
  result=pthread_join(thid2,&ret);   printf("thid2 result=%d,ret=%ld\n",result,ret);
  result=pthread_join(thid1,&ret);   printf("thid1 result=%d,ret=%ld\n",result,ret);
  printf("join ok.\n");
}

void *thmain1(void *arg)    // 线程1主函数。
{
  for (int ii=0;ii<3;ii++)
  {
    sleep(1); printf("pthmain1 sleep(%d) ok.\n",ii+1);
  }
  return (void *) 1;
}

void *thmain2(void *arg)    // 线程2主函数。
{
  for (int ii=0;ii<5;ii++)
  {
    sleep(1); printf("pthmain2 sleep(%d) ok.\n",ii+1);
  }
  return (void *) 2;
}
