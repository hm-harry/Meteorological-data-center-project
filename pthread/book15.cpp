// 本程序演示线程同步-条件变量。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

pthread_cond_t cond=PTHREAD_COND_INITIALIZER;     // 声明条件变量并初始化。
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;  // 声明互斥锁并初始化。

void *thmain(void *arg);    // 线程主函数。

void handle(int sig);       // 信号15的处理函数。

int main(int argc,char *argv[])
{
  signal(15,handle);       // 设置信号15的处理函数。

  pthread_t thid1,thid2,thid3;

  // 创建线程。
  if (pthread_create(&thid1,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
  if (pthread_create(&thid2,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
  if (pthread_create(&thid3,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }

  // 等待子线程退出。
  pthread_join(thid1,NULL);  pthread_join(thid2,NULL);  pthread_join(thid3,NULL);  

  pthread_cond_destroy(&cond);    // 销毁条件变量。
  pthread_mutex_destroy(&mutex);  // 销毁互斥锁。
}

void *thmain(void *arg)    // 线程主函数。
{
  while (true)
  {
    printf("线程%lu开始等待条件信号...\n",pthread_self());
    pthread_cond_wait(&cond,&mutex);    // 等待条件信号。
    printf("线程%lu等待条件信号成功。\n\n",pthread_self());
  }
}

void handle(int sig)       // 信号15的处理函数。
{
  printf("发送条件信号...\n");
  // pthread_cond_signal(&cond);      // 唤醒等待条件变量的一个线程。
  pthread_cond_broadcast(&cond);   // 唤醒等待条件变量的全部线程。
}
