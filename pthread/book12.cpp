// 本程序演示线程同步-互斥锁。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int var;

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;      // 声明互斥锁。

void *thmain(void *arg);    // 线程主函数。

int main(int argc,char *argv[])
{
  // pthread_mutex_init(&mutex,NULL);   // 初始化互斥锁。

  pthread_t thid1,thid2;

  // 创建线程。
  if (pthread_create(&thid1,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
  if (pthread_create(&thid2,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }

  // 等待子线程退出。
  printf("join...\n");
  pthread_join(thid1,NULL);  
  pthread_join(thid2,NULL);  
  printf("join ok.\n");

  printf("var=%d\n",var);

  pthread_mutex_destroy(&mutex);  // 销毁锁。
}

void *thmain(void *arg)    // 线程主函数。
{
  for (int ii=0;ii<1000000;ii++)
  {
    pthread_mutex_lock(&mutex);    // 加锁。
    var++;
    pthread_mutex_unlock(&mutex);  // 解锁。
  }
}
