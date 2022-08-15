// 本程序演示线程参数的传递（用结构体的地址传递多个参数）。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void *thmain(void *arg);    // 线程的主函数。

struct st_args
{
  int  no;        // 线程编号。
  char name[51];  // 线程名。
};

int main(int argc,char *argv[])
{
  pthread_t thid=0;

  // 创建线程。
  struct st_args *stargs=new struct st_args;
  stargs->no=15;   strcpy(stargs->name,"测试线程");
  if (pthread_create(&thid,NULL,thmain,stargs)!=0) { printf("pthread_create failed.\n"); exit(-1); }

  // 等待子线程退出。
  printf("join...\n");
  pthread_join(thid,NULL);  
  printf("join ok.\n");
}

void *thmain(void *arg)    // 线程主函数。
{
  struct st_args *pst=(struct st_args *)arg;
  printf("no=%d,name=%s\n",pst->no,pst->name);
  delete pst;
  printf("线程开始运行。\n");
}

