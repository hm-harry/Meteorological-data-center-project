// 本程序演示线程资源的回收（线程清理函数）。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void *thmain(void *arg);    // 线程主函数。

void  thcleanup1(void *arg);    // 线程清理函数1。
void  thcleanup2(void *arg);    // 线程清理函数2。
void  thcleanup3(void *arg);    // 线程清理函数3。

int main(int argc,char *argv[])
{
  pthread_t thid;

  // 创建线程。
  if (pthread_create(&thid,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }

  int result=0;
  void *ret;
  printf("join...\n");
  result=pthread_join(thid,&ret);   printf("thid result=%d,ret=%ld\n",result,ret);
  printf("join ok.\n");
}

void *thmain(void *arg)    // 线程主函数。
{
  pthread_cleanup_push(thcleanup1,NULL);  // 把线程清理函数1入栈（关闭文件指针）。
  pthread_cleanup_push(thcleanup2,NULL);  // 把线程清理函数2入栈（关闭socket）。
  pthread_cleanup_push(thcleanup3,NULL);  // 把线程清理函数3入栈（回滚数据库事务）。 

  for (int ii=0;ii<3;ii++)
  {
    sleep(1); printf("pthmain sleep(%d) ok.\n",ii+1);
  }

  pthread_cleanup_pop(3);  // 把线程清理函数3出栈。
  pthread_cleanup_pop(2);  // 把线程清理函数2出栈。
  pthread_cleanup_pop(1);  // 把线程清理函数1出栈。
}

void  thcleanup1(void *arg)
{
  // 在这里释放关闭文件、断开网络连接、回滚数据库事务、释放锁等等。
  printf("cleanup1 ok.\n");
};

void  thcleanup2(void *arg)
{
  // 在这里释放关闭文件、断开网络连接、回滚数据库事务、释放锁等等。
  printf("cleanup2 ok.\n");
};

void  thcleanup3(void *arg)
{
  // 在这里释放关闭文件、断开网络连接、回滚数据库事务、释放锁等等。
  printf("cleanup3 ok.\n");
};

