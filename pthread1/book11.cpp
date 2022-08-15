// 本程序演示线程安全。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <atomic>
#include <iostream>

std::atomic<int> var;

void *thmain(void *arg);    // 线程主函数。

int main(int argc,char *argv[])
{
  pthread_t thid1,thid2;

  // 创建线程。
  if (pthread_create(&thid1,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
  if (pthread_create(&thid2,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }

  // 等待子线程退出。
  printf("join...\n");
  pthread_join(thid1,NULL);  
  pthread_join(thid2,NULL);  
  printf("join ok.\n");

  // printf("var=%d\n",var);
  std::cout << "var=" << var << std::endl;
}

void *thmain(void *arg)    // 线程主函数。
{
  for (int ii=0;ii<1000000;ii++)
  {
    var++;
    // __sync_fetch_and_add(&var,1);
  }
}
