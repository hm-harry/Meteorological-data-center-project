// 本程序演示线程同步-信号量。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int var;

sem_t sem;      // 声明信号量。

void *thmain(void *arg);    // 线程主函数。

int main(int argc,char *argv[])
{
  sem_init(&sem,0,1);   // 初始化信号量。

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

  sem_destroy(&sem);  // 销毁信号量。
}

void *thmain(void *arg)    // 线程主函数。
{
  for (int ii=0;ii<1000000;ii++)
  {
    sem_wait(&sem);    // 加锁。
    var++;
    sem_post(&sem);  // 解锁。
  }
}
