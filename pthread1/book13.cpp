// 本程序演示线程同步-自旋锁。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int var;

pthread_spinlock_t spin;      // 声明自旋锁。

void *thmain(void *arg);    // 线程主函数。

int main(int argc,char *argv[])
{
  pthread_spin_init(&spin,PTHREAD_PROCESS_PRIVATE);   // 初始化自旋锁。

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

  pthread_spin_destroy(&spin);  // 销毁锁。
}

void *thmain(void *arg)    // 线程主函数。
{
  for (int ii=0;ii<1000000;ii++)
  {
    pthread_spin_lock(&spin);    // 加锁。
    var++;
    pthread_spin_unlock(&spin);  // 解锁。
  }
}
