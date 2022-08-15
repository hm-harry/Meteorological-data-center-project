// 本程序演示线程的创建和终止。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void* thmain1(void* arg); // 线程主函数
void* thmain2(void* arg); // 线程主函数

int var = 0;
pthread_t thid2 = 0;

int main(int argc,char *argv[]){
  pthread_t thid1 = 0; // 线程id typdef unsigned long pthread_t
  

  // 创建线程
  if(pthread_create(&thid1, NULL, thmain1, NULL) != 0){
    printf("pthread_create failed.\n");
    exit(-1);
  }

  if(pthread_create(&thid2, NULL, thmain2, NULL) != 0){
    printf("pthread_create failed.\n");
    exit(-1);
  }

  // sleep(2);
  // pthread_cancel(thid1);
  // sleep(2);
  // pthread_cancel(thid2);

  printf("join...\n");
  pthread_join(thid1, NULL);
  pthread_join(thid2, NULL);
  printf("join ok\n");
}

void* thmain1(void* arg){ // 线程主函数
  for(int ii = 0; ii < 5; ++ii){
    var = ii + 1;
    sleep(1);
    printf("pthmain1 sleep(%d) ok.\n", var);
    if(ii == 2) pthread_cancel(thid2);
  }
}

void* thmain2(void* arg){ // 线程主函数
  for(int ii = 0; ii < 5; ++ii){
    sleep(1);
    printf("pthmain2 sleep(%d) ok.\n", var);
    // if(ii == 2) return 0;
  }
}