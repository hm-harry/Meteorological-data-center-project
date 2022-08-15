// 本程序演示线程同步-条件变量，pthread_cond_wait(&cond,&mutex)函数中发生了什么？
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

pthread_cond_t cond=PTHREAD_COND_INITIALIZER;     // 声明条件变量并初始化。
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;  // 声明互斥锁并初始化。

void *thmain1(void *arg);    // 线程1主函数。
void *thmain2(void *arg);    // 线程2主函数。

int main(int argc,char *argv[])
{
  pthread_t thid1,thid2;

  // 创建线程。
  if (pthread_create(&thid1,NULL,thmain1,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
sleep(1);
  if (pthread_create(&thid2,NULL,thmain2,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }

  // 等待子线程退出。
  pthread_join(thid1,NULL);  pthread_join(thid2,NULL);  

  pthread_cond_destroy(&cond);    // 销毁条件变量。
  pthread_mutex_destroy(&mutex);  // 销毁互斥锁。
}

void *thmain1(void *arg)    // 线程1的主函数。
{
  printf("线程一申请互斥锁...\n");
  pthread_mutex_lock(&mutex);
  printf("线程一申请互斥锁成功。\n");

  printf("线程一开始等待条件信号...\n");
  pthread_cond_wait(&cond,&mutex);    // 等待条件信号。
  printf("线程一等待条件信号成功。\n");
}

void *thmain2(void *arg)    // 线程2的主函数。
{
  printf("线程二申请互斥锁...\n");
  pthread_mutex_lock(&mutex);
  printf("线程二申请互斥锁成功。\n");

  pthread_cond_signal(&cond); 

  sleep(5);

  printf("线程二解锁。\n");
  pthread_mutex_unlock(&mutex);

  return 0;

  printf("线程二申请互斥锁...\n");
  pthread_mutex_lock(&mutex);
  printf("线程二申请互斥锁成功。\n");

  printf("线程二开始等待条件信号...\n");
  pthread_cond_wait(&cond,&mutex);    // 等待条件信号。
  printf("线程二等待条件信号成功。\n");
}

