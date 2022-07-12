#include<_public.h>

// 程序运行的日志
CLogFile logfile;

int main(int argc, char* argv[]){
  //程序的帮助
  if(argc != 2){
    printf("\n");
    printf("Using:./checkproc logfilename\n");
    printf("Example:/project/tools/bin/procctl 10 /project/tools1/bin/checkproc /tmp/log/checkproc.log\n\n");
    printf("本程序用于检查后台服务程序是否超时，如果已经超时，就终止它。\n");
    printf("注意：\n");
    printf("1) 本程序由procctl启动，运行周期建议10秒。\n");
    printf("2) 为了避免被普通用户误杀，本程序应该由root用户启动。\n\n");
    printf("3) 如果要终止本程序，只能killall -9 终止。\n\n\n");

    return 0;
  }

  // 忽略全部信号不希望程序被干扰
  //for(int ii = 1; ii <= 64; ++ii) signal(ii, SIG_IGN);
  CloseIOAndSignal(true);

  // 打开日志
  if(logfile.Open(argv[1], "a+") == false){
    printf("logfile.Open(%s) failed.\n", argv[1]);
    return -1;
  }

  int shmid = 0;

  // 创建/获取共享内存
  if((shmid = shmget((key_t)SHMKEYP, MAXNUMP * sizeof(struct st_procinfo), 0666|IPC_CREAT)) == -1){
    logfile.Write("创建/获取共享内存(%x)失败。\n", SHMKEYP);
    return false;
  }

  // 将共享内存连接到当前进程的地址空间
  struct st_procinfo* shm = (struct st_procinfo*)shmat(shmid, 0, 0);

  // 遍历共享内存中全部记录
  for(int ii = 0; ii < MAXNUMP; ++ii){
    //如果记录的pid==0，表示空记录，continue；
    if(shm[ii].pid == 0) continue;

    // 如果记录的pid!=0，表示是服务程序的心跳记录
    // logfile.Write("ii = %d, pid = %d, pname = %s, timeout = %d, atime = %d\n", ii, shm[ii].pid, shm[ii].pname, shm[ii].timeout, shm[ii].atime);

    // 向进程发送信号0，判断它是否还存在，如果不存在，从共享内存中删除该记录，continue;
    int iret = kill(shm[ii].pid, 0);//进程存在返回0 不存在返回-1
    if(iret == -1){
      logfile.Write("进程pid = %d (%s)已经不存在。\n", (shm + ii)->pid, (shm + ii)->pname);
      memset(shm + ii, 0, sizeof(struct st_procinfo)); // 从共享内存中删除该记录
      continue;
    }

    time_t now = time(0); // 获取当前时间
    // 如果进程未超时，contunue;
    if(now - shm[ii].atime < shm[ii].timeout) continue;

    // 如果已经超时
    logfile.Write("进程pid = %d (%s)已经超时。\n", (shm + ii)->pid, (shm + ii)->pname);

    // 发送15的信号尝试终止进程
    kill(shm[ii].pid, 15);

    // 每隔1秒判断一次进程是否终止，累计5秒，一般来说，5秒的时间是足够让进程退出
    for(int jj = 0; jj < 5; ++jj){
      sleep(1);
      iret = kill(shm[ii].pid, 0);// 判断进程是否存在
      if(iret == -1) break;
    }

    // 如果进程仍存在，就发送信号9，强制终止它
    if(iret == -1)
      logfile.Write("进程pid = %d (%s)已经正常终止。\n", (shm + ii)->pid, (shm + ii)->pname);
    else{
      kill(shm[ii].pid, 9);
      logfile.Write("进程pid = %d (%s)已经强制终止。\n", (shm + ii)->pid, (shm + ii)->pname);
    }

    // 从共享内存中删除已经超时的心跳记录
    memset(shm + ii, 0, sizeof(struct st_procinfo));
  }

  // 把共享内存从当前进程中分离
  shmdt(shm);
  return 0;
}
