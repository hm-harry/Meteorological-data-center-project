// 本程序演示线程同步-读写锁。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

pthread_rwlock_t rwlock=PTHREAD_RWLOCK_INITIALIZER;   // 声明读写锁并初始化。

void *thmain(void *arg);    // 线程主函数。

void handle(int sig);       // 信号15的处理函数。

int main(int argc,char *argv[])
{
  signal(15,handle);       // 设置信号15的处理函数。

  pthread_t thid1,thid2,thid3;

  // 创建线程。
  if (pthread_create(&thid1,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
sleep(1);
  if (pthread_create(&thid2,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
sleep(1);
  if (pthread_create(&thid3,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }

  // 等待子线程退出。
  pthread_join(thid1,NULL);  pthread_join(thid2,NULL);  pthread_join(thid3,NULL);  

  pthread_rwlock_destroy(&rwlock);  // 销毁锁。
}

void *thmain(void *arg)    // 线程主函数。
{
  for (int ii=0;ii<100;ii++)
  {
    printf("线程%lu开始申请读锁...\n",pthread_self());
    pthread_rwlock_rdlock(&rwlock);    // 加锁。
    printf("线程%lu开始申请读锁成功。\n\n",pthread_self());
    sleep(5);
    pthread_rwlock_unlock(&rwlock);    // 解锁。
    printf("线程%lu已释放读锁。\n\n",pthread_self());

    if (ii==3) sleep(8);
  }
}

void handle(int sig)       // 信号15的处理函数。
{
  printf("开始申请写锁...\n");
  pthread_rwlock_wrlock(&rwlock);    // 加锁。
  printf("申请写锁成功。\n\n");
  sleep(10);
  pthread_rwlock_unlock(&rwlock);    // 解锁。
  printf("写锁已释放。\n\n");
}
