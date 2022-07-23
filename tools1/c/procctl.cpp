#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>


int main(int argc, char* argv[]){

  if(argc < 3){
    printf("Using:./procctl timetv1 program argv ...\n");
    printf("Example:/project/tools1/bin/procctl 5 /usr/bin/tar zcvf /tmp/tmp.tgz /usr/include\n\n");

    printf("本程序是服务程序的调度程序，周期性启动服务程序或shell脚本。\n");
    printf("timetvl 运行周期，单位：秒。被调度的程序运行结束后，在timetvl秒后会被procctl重新启动。\n");
    printf("program 被调度的程序名，必须使用全路径。\n");
    printf("argvs   被调度的程序的参数。\n");
    printf("注意，本程序不会被kill杀死，但可以用kill -9强行杀死。\n\n\n");

    return -1;
  }
  // 先执行fork函数，创建一个子进程，让子进程调用execl执行新的程序
  // 新程序将替换子进程，不会影响父进程
  // 在父进程中，可以调用wait函数等待新程序运行的结果，这样就可以实现调度的功能
  //
  // exec是用参数中指定程序替换了当前进程的正文段、数据段、堆和栈
  // 成功调用之后覆盖了现有进程

  // 关闭信号和IO，本程序不希望被打扰。
  for(int ii = 0; ii < 64; ++ii){
    signal(ii, SIG_IGN);
    close(ii);
  }

  // 生成子进程，父进程退出，让程序运行在后台，由系统1号进程托管。
  if(fork()!=0) exit(0);

  signal(SIGCHLD, SIG_DFL);

  char* pargv[argc];
  for(int ii = 2; ii < argc; ++ii){
    pargv[ii - 2] = argv[ii];
  }
  pargv[argc - 2] = NULL;
  while(true){
    if(fork() == 0){
       // execl("/usr/bin/ls", "/usr/bin/ls", "-lt", "/tmp", (char*)0);
       execv(argv[2], pargv);
       exit(0);
    }else{
      int status;
      wait(&status);
      sleep(atoi(argv[1]));
    }
  }
  return 0;
}
