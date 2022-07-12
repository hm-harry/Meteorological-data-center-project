// 本程序演示用互斥锁和条件变量实现高速缓存。
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <vector>

using namespace std;

// 缓存队列消息的结构体。
struct st_message
{
  int  mesgid;          // 消息的id。
  char message[1024];   // 消息的内容。
}stmesg;

vector<struct st_message> vcache;  // 用vector容器做缓存。

pthread_cond_t  cond=PTHREAD_COND_INITIALIZER;     // 声明并初始化条件变量。
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;   // 声明并初始化互斥锁。

void  incache(int sig);      // 生产者、数据入队。
void *outcache(void *arg);   // 消费者、数据出队线程的主函数。

int main()
{
  signal(15,incache);  // 接收15的信号，调用生产者函数。

  // 创建三个消费者线程。
  pthread_t thid1,thid2,thid3;
  pthread_create(&thid1,NULL,outcache,NULL);
  pthread_create(&thid2,NULL,outcache,NULL);
  pthread_create(&thid3,NULL,outcache,NULL);

  pthread_join(thid1,NULL);
  pthread_join(thid2,NULL);
  pthread_join(thid3,NULL);

  pthread_cond_destroy(&cond);
  pthread_mutex_destroy(&mutex);

  return 0;
}

void incache(int sig)       // 生产者、数据入队。
{
  static int mesgid=1;  // 消息的计数器。

  struct st_message stmesg;      // 消息内容。
  memset(&stmesg,0,sizeof(struct st_message));

  pthread_mutex_lock(&mutex);    // 给缓存队列加锁。

  //  生产数据，放入缓存队列。
  stmesg.mesgid=mesgid++; vcache.push_back(stmesg);  
  stmesg.mesgid=mesgid++; vcache.push_back(stmesg);  
  stmesg.mesgid=mesgid++; vcache.push_back(stmesg);  
  stmesg.mesgid=mesgid++; vcache.push_back(stmesg);  

  pthread_mutex_unlock(&mutex);  // 给缓存队列解锁。

  //pthread_cond_signal(&cond);    // 发送条件信号，激活一个线程。
  pthread_cond_broadcast(&cond); // 发送条件信号，激活全部的线程。
}

void  thcleanup(void *arg)
{
  // 在这里释放关闭文件、断开网络连接、回滚数据库事务、释放锁等等。
  printf("cleanup ok.\n");

  pthread_mutex_unlock(&mutex);

  /*
  A condition  wait  (whether  timed  or  not)  is  a  cancellation  point. When the cancelability type of a thread is set to PTHREAD_CAN_CEL_DEFERRED, a side-effect of acting upon a cancellation request while in a condition wait is that the mutex is (in effect)  re-acquired before  calling the first cancellation cleanup handler. The effect is as if the thread were unblocked, allowed to execute up to the point of returning from the call to pthread_cond_timedwait() or pthread_cond_wait(), but at that point notices  the  cancellation  request  and instead  of  returning to the caller of pthread_cond_timedwait() or pthread_cond_wait(), starts the thread cancellation activities, which includes calling cancellation cleanup handlers.
  意思就是在pthread_cond_wait时执行pthread_cancel后，
  要先在线程清理函数中要先解锁已与相应条件变量绑定的mutex，
  这样是为了保证pthread_cond_wait可以返回到调用线程。
  */
};

void *outcache(void *arg)    // 消费者、数据出队线程的主函数。
{
  pthread_cleanup_push(thcleanup,NULL);  // 把线程清理函数入栈。

  struct st_message stmesg;  // 用于存放出队的消息。

  while (true)
  {
    pthread_mutex_lock(&mutex);  // 给缓存队列加锁。

    // 如果缓存队列为空，等待，用while防止条件变量虚假唤醒。
    while (vcache.size()==0)
    {
      pthread_cond_wait(&cond,&mutex);
    }

    // 从缓存队列中获取第一条记录，然后删除该记录。
    memcpy(&stmesg,&vcache[0],sizeof(struct st_message)); // 内存拷贝。
    vcache.erase(vcache.begin());

    pthread_mutex_unlock(&mutex);  // 给缓存队列解锁。

    // 以下是处理业务的代码。
    printf("phid=%ld,mesgid=%d\n",pthread_self(),stmesg.mesgid);
    usleep(100);
  }

  pthread_cleanup_pop(1);  // 把线程清理函数出栈。
}

