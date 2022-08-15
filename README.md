# 气象数据中心实战项目

[TOC]



## 项目介绍

项目依托气象行业数据（数据总量大，且每天新增数据量大），模拟气象数据中心

该项目从数据源采集数据后存储到多个数据库中；业务系统通过数据库集群的总线访问数据

主要功能子系统为：

**1.数据采集子系统**

功能：把数据从数据源取出

三个模块：（1）ftp客户端，直接下载数据文件；（2）http客户端，采用http协议，从WEB服务接口数据；（3）直连数据库的数据源，从表中抽取数据

**2.数据处理和加工子系统**

功能：把各种格式的原始数据解码为xml格式的数据文件；对原始数据进行二次加工生成高可用的数据集。

**3.数据入库子系统**

功能：把数百种数据存储到数据中心的表中

**4.数据同步子系统**

MySQL的高可用方案只能解决部分问题，且不够灵活，效率不高（只能满足数据库之间的复制不能满足表之间的复制）。本项目把核心数据库(Slave)表中的数据按条件和数据增量两种方式同步到业务数据库中。

项目数据库主要分为两种Master和Slave。

业务数据库主要由业务的需求决定（历史库（量大效率低）， 实时库（量小效率高）， 预报库， 台风库（量小效率高）， 接口库）

**5.数据管理子系统**

功能：清理历史数据；把历史数据备份和归档

**6.数据交换子系统**

功能：把数据中心的数据从表中导出来生成数据文件。采用ftp协议，把数据文件推送到对方的ftp服务器。基于tcp协议的快速传输文件子系统

**7.数据服务总线子系统**

功能：用C++开发WEB服务，为业务提供数据访问接口，使用数据库连接池、线程池效率极高

**8.网络代理服务子系统**

功能：用于运维，利用I/O复用技术(select/poll/epoll)

**项目重点和难点：**（1）服务程序的稳定性；（2）数据处理和服务的效率；（3）功能模块的通用性



## 开发环境

CentOS7.x	MySQL5.7	字符集utf-8



## 项目框架

 

![未命名文件](%E6%9C%AA%E5%91%BD%E5%90%8D%E6%96%87%E4%BB%B6.png)



## 服务程序常驻后台

保持服务程序稳定性，用守护进程监控服务程序的运行状态，如果服务程序故障，调度进程会重启服务程序



### 服务程序的调度

利用linux信号和多进程

先执行fork函数，创建一个子进程，让子进程调用execl执行新的程序

新程序将替换子进程，不会影响父进程

 在父进程中，可以调用wait函数等待新程序运行的结果，这样就可以实现调度的功能



### 守护进程的实现

利用linux共享内存和linux信号量

// 查看共享内存：  ipcs -m   删除共享内存：  ipcrm -m shmid

// 查看信号量：    ipcs -s  删除信号量：    ipcrm sem semid

**信号量CSEM m_sem;** 

m_semid=semget(key,1,0666)

如果errno==2表示信号量不存在， 创建m_semid=**semget**(key,1,0666|IPC_CREAT|IPC_EXCL)

// 信号量创建成功后，还需要把它初始化成value。

union semun sem_union;		sem_union.val = value;   // 设置信号量的初始**semctl**(m_semid,0,SETVAL,sem_union)

// **加锁** CSEM::P 调用：

**semop**(m_semid,&sem_b,1)   其中：struct **sembuf** sem_b;    sem_b.sem_num = 0; sem_b.sem_op = -1; sem_b.sem_flg = m_sem_flg;



**共享内存**

**创建/获取**共享内存，键值为SHMKEYP，大小为MAXNUMP个st_procinfo结构体的大小。

**shmget**：m_shmid = shmget((key_t)SHMKEYP, MAXNUMP*sizeof(struct st_procinfo), 0666|IPC_CREAT)

将共享内存**链接**到当前进程的地址空间。

**shmat**： m_shm=(struct st_procinfo *)shmat(m_shmid, 0, 0);

把共享内存从当前进程中**分离**。

**shmdt** ：shmdt(m_shm)



## 数据采集子系统&数据交换子系统

### ftp客户端下载模块

模块功能：已知服务器的文件路径名和保存到本地的文件路径名，**把服务器上某个目录的文件全部下载到本地目录**（可以指定文件名匹配规则）。文件下载成功后，ptype==1，服务器啥都不做，ptype==2，服务器删除文件，ptype==3，服务器备份文件

模块位置：/project/tools1/c/ftpgetfiles.cpp

模块实现：

1. 基于Cftp类，头文件在/project/public/_ftp.h中

   结构体st_fileinfo中包含文件名filename，文件时间mtime

   容器vlistfile2存放服务器待下载文件信息；容器vlistfile1存放已经传输完成的文件信息；

   容器vlistfile3存放本地以及下载的但服务器中还有的文件；容器vlistfile4存放服务器中新增文件

   listfilename中存放服务器文件；okfilename中存放已经传输完成的文件；strremotefilenamebak备份目录

2. 输入参数写成xml格式方便应对众多输入参数。利用**GetXMLBuffer**函数读取输入参数

2. 创建心跳信息：**CPActive**类中的**AddPInfo**(starg.timeout, starg.pname)函数

3. 登录ftp服务器：**ftp.login**(starg.host, starg.username, starg.password, starg.mode)

4. 进入ftp服务器存放文件的目录**ftp.chdir**(starg.remotepath)，（调用FtpChdir）

5. 调用**ftp.nlist()**方法列出服务器目录中的文件，结果存放到本地文件listfilename中：

   ftp.nlist(".", starg.listfilename)，其中有MKDIR()和FtpNlst(listfilename,remotedir,m_ftpconn)

6. LoadListFile加载文件函数，将上述ftp.nlist()方法获取到的list文件加载到容器vlistfile2中（1）打开本地文件File.Open（listfilename）（2）读取本地文件中的一行到File.Fgets（3）MatchStr匹配满足条件的文件（4）当ptype为1，checktime为true时，调用ftp.mtime（底层调用FtpModDate函数）获取ftp服务器文件时间放入ftp.m_mtime中

7. 如果ptype==1（1）加载okfilename文件中的内容到容器vlistfile1中（2）比较vlistfile2和vlistfile1，得到vlistfile3和vlistfile4（3）把vlistfile4中的内容复制到vlistfile2中：vlistfile2.clear(); vlistfile2.swap(vlistfile4);

8. 遍历容器vlistfile2，调用**ftp.get()**方法从服务器下载文件：ftp.get(strremotefilename, strlocalfilename)

9. 如果ptype==1，把下载成功的文件记录追加到okfilename文件中

10. 如果ptype==2，使用ftp.ftpdelete()删除服务器文件

11. 如果ptype==3，使用ftp.ftprename()服务器文件转存到服务器备份目录



### ftp客户端下载模块



## 数据处理和统计系统

数据处理：

任务：把各种格式的数据文件转换为xml格式的文件，再交给入库程序

步骤：1. 读取目录中的文件 2.解析文件的内容 3.生成xml文件格式的文件

数据统计：

任务：把采集到的数据进行二次加工，生成新的数据产品，最大化数据价值



## 数据入库子系统





## 数据同步子系统

MySQL高可用方案的不足：

1.Master（写）和Slave（读）的表结构和数量必须一致

2.非主从关系的数据库不能进行复制

3.不够灵活，效率也不高



我们的做法：

基于MySQL的Federated存储引擎（Oracle dblink）把远程数据库同步到本地（实现数据库中的互相访问）

### 刷新同步模块

针对需要覆盖本地表的情况，开发了**刷新同步模块**，模块位置：/project/tools1/c/syncupdate.cpp

模块实现：

若需要同步的数据量少，采用一次性全部同步：1. delete from (localtname) (where) 删除本地表中的数据 ；2. insert into (localtname)(localcols) select (remotecols) from (fedtname) (where)把Federated表中满足where条件的记录插入到本地表中

若需要同步的数据量少，采用分批同步：1.select (remotekeycol) from (remotetname) (where)从远程表中查找需要同步记录的key字段的值；2.delete from (localtname) where (localkeycol) in (:1, :2, :3, ...maxcount) 根据key字段一次性从本地表删除starg.maxcount条数据；3.insert into  (localtname)(localcols) select (remotecols) from (fedtname) where (remotekeycol) in  (:1, :2, :3, ...maxcount)插入本地表数组的SQL语句，一次插入starg.maxcount条记录

每次执行select语句取出需要删除的remotekeycol，放入delete和insert绑定的输入数组中，达到一定数量(maxcount) 执行delete和insert语句



### 增量同步模块

针对需要**不**覆盖本地表的情况，开发了**增量同步模块**，模块位置：/project/tools1/c/syncincrement.cpp

模块实现：

与刷新同步中的分批同步类似，这里select中需要查找本地最大的id从远程收集比这个id更大的数据加入delete和select参数



问题：

1.面对远程表的删除操作，本程序无法使用

解决办法：远程表上建立触发器，用触发器表同步本地表和远程表；或者增加一个程序，定期扫描远程表和本地表，如果记录在本地表中有但是远程表没有就删除他

2.数据略有延迟，但是在业务可接受范围内

3.对远程数据库会造成压力。解决办法：捕捉Binlog日志，不用读MySQL数据库，直接读日志就行



## 数据管理子系统

系统功能：1.删除表中符合条件数据；2.把符合条件的历史数据备份和归档

### 数据清理模块

模块位置：/project/tools1/c/deletetable.cpp

模块功能：根据清理数据的条件，把表中的唯一键字段查询出来；以唯一键为条件，删除表中的记录；为了提高效率每执行一条SQL语句删除100或200条数据。

模块实现：1.select （keycol） from （tname） （where ddatetime < timestampadd(minute, -10, now())）每次取出一条满足条件的keycol；2.delete from （tname） where （keycol） in (bindstr)；3.字符串数组绑定到bindstr；4.读入bindstr，每MAXPARAMS次执行一次delete



### 数据迁移模块

模块位置：/project/tools1/c/migratetable.cpp

模块功能：迁移表中的数据

模块实现：在数据清理模块的删除操作之前 执行

insert into dsttname(m_allcols) select m_allcols from srctname where keycol in (bindstr)



## Oracle数据开发

 sqlplus / as sysdba

### db link

类似于mysql的Federated引擎，创建方法是

1.超级用户登录授权 grant create database link to scott; 2.登录scott用户数据库create database link 数据库链路名 connect to 某用户名 identified by 某用户密码 using '远程数据库连接';



### Oracle数据库集群方案

RAC（Real Application Cluster实时应用集群）由多个服务器加共享存储，一般两个节点，高可用方案，写入数据的性能比单实例略低，读略高。



Data Guard数据同步，把日志文件从源数据库传到目标数据库，从而达到数据库级别的高可用方案，相当于mysql的主从复制，mysql叫master和slaver，oracle叫primary和standby，用于异地备份，读写分离



OGG（Oracle Golden Gate），捕捉远端数据库的日志，把发生变化的数据提取出来，生成文件发送给目标端，目标端解析文件，数据同步效率比较高，非常灵活，可以配置需要同步的表



OGG Vs 数据同步子系统

1.OGG收费

2.OGG从日志中抽取，因此对源数据库没有压力，对表的设计没有要求。但数据同步子系统要求要有自增字段，且不能插入删除

3.数据同步子系统部署简单，批量操作数据效率更高



RAC服务器采用IBM3850内存：128G，硬盘：300G

存储服务器EMC 100T

应用服务器IBM3650



Oracle DBA（database admin）

1.数据库建设方案的设计和实施 2.数据库日常运维，数据安全，性能分析，持续优化

3.数据库升级迁移，备份，扩容，容灾的设计

程序员不只是CRUD，充分利用索引，数据的复制和迁移可以利用数据同步子系统，不用生成临时文件



## Linux 线程

进程：拥有PCB（进程管理块：系统中存放进程的管理和控制信息的数据结构），**有独立的地址空间**

线程：轻量级的进程，本质仍是进程，操作系统底层都是调用clone，如果是线程就不复制地址空间，和原来的进程共享地址空间，**无独立地址空间**

线程是cpu最小的执行和调度单位，进程是最小的分配资源单位

**线程的优点**：可以在一个进程内实现并发，开销少，数据通信/共享方便

### 线程的创建和终止

线程的创建：int **pthread_create**(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg);

线程退出：int **pthread_join**(pthread_t thread, void **retval);

线程非正常终止：

1.主线程终止，全部线程将强行终止 2.子进程调用exit()终止整个进程

多进程中子进程core dump不会影响其他进程，多线程子进程core dump整个进程core dump

正常退出：1.返回 2.线程可以被同一进程中的其他线程调用**pthread_cancel**(pthread_t *thread)退出 3.在线程中调用**pthread_exit** (void *返回值)退出，返回值会传递给**pthread_join**（thid,(void **)）第二个参数



### 线程参数的传递

var=1;if (pthread_create(&thid1,NULL,thmain1,**(void *)(long)**var)!=0)

void *thmain1(void *arg)    // 线程主函数。

{printf("var1=%d\n",**(int)(long)**arg);printf("线程1开始运行。\n");}



在线程中调用**pthread_exit** (void *返回值)退出，返回值会传递给**pthread_join**（thid,(void **)）第二个参数



### 线程资源的回收

线程有非分离（joinable）和分离(unjoinable)两种状态

缺省状态是非分离（joinable）或者是可连接的

线程的分离，把线程属性设置为分离，线程结束之后，系统回收资源，调用**pthread_detach**(**pthread_self**())分离线程

非阻塞：pthread_tryjoin_np() 限时阻塞等待pthread_timedjoin_np

**线程清理函数**：清理函数入栈**pthread_cleanup_push**(线程清理函数名, 函数参数)  清理函数出栈**pthread_cleanup_pop**(1执行函数，2不执行)            线程清理函数：void ...(void *)

**进程清理函数：** atexit();



### 线程的取消，线程与信号

调用pthread_cancel()取消

线程被取消后，返回状态是PTHREAD_CANCELED,即-1

设置线程的取消状态pthread_setcancelstate(PTHREAD_CANCEL_ENABLE，NULL);

设置线程的取消方式pthread_setcanceltype();  

PTHREAD_CANCEL_DEFERRED默认到达取消点取消，PTHREAD_CANCEL_ASYNCHRONOUS立即取消

pthread_testcancel();设置取消点



在多线程中外部信号，向进程发送信号不会中断系统调用      但是多线程程序会中断

在多线程中，信号的处理是所有线程共享的，以最后出现为准

进程中的信号可以送达单个线程会中断系统调用，**pthread_kill**(pthread_t thread, int sig);

如果某个线程因为信号终止，整个进程终止



### 线程安全

原子性：一个操作要么全部执行，要么全部不执行    CPU执行指令：读取指令，读取内存，执行指令，写回内存。

可见性：当多个线程并发访问共享变量时，一个线程对共享变量的修改，其他线程能立即看到。但cpu有缓存。。

顺序性：CPU会优化代码的顺序。

**如何解决：**

volatile关键字，保证变量的内存可见性，禁止代码优化，但不是**原子的**。

原子操作：本质是线程锁，三条汇编指令：xadd、cmpxchg、xchg。__sync_with_fatch_and_add(type* ptr,val )

原子变量：**atomic<T>**              编译时加上-std=c11 

**线程同步：**。。。



### 线程同步，锁

**互斥锁**：加锁和解锁 ，适用于等待时间可能比较长的场景

**pthread_mutex_t** mutex;  **pthread_mutex_init**()相当于PTHREAD_MUTEX_INITIALIZER; **pthread_mutex_lock**(); **pthread_mutex_unlock**();pthread_mutex_t destory()



**自旋锁：**设置一个循环不断检测，**适用于等待时间很短的场景**

**pthread_spinlock_t** mutex; **pthread_spin_init**(); **pthread_spin_lock**(); **pthread_spin_unlock**();

PTHREAD_PROCESS_PRIVATE/PTHREAD_PROCESS_SHARED 不同进程中的线程是否可以共享自旋锁



**读写锁**：允许更高的并发性，三种状态：读模式加锁，写模式加锁，不加锁，**适用于读的次数远大于写**

只要没有进程持有写锁，任意线程都可以申请读锁，只有在不加锁的情况下，才能成功申请写锁

pthread_rwlock_rdlock(); pthread_rwlock_tryrdlock(); pthread_rwlock_timedrdlock();

pthread_rwlock_wrlock(); pthread_rwlock_trywrlock(); pthread_rwlock_timedwrlock();

pthread_rwlock_unlock();



**条件变量**：与互斥锁一起使用，实现通知功能

**pthread_cond_t** cond; pthread_cond_init(); pthread_cond_destroy(); 等待唤醒：**pthread_cond_wait**(); pthread_cond_timedwait(); 唤醒一个线程：**pthread_cond_signal**();唤醒所有：pthread_cond_broadcast();

共享属性：获取：pthread_condattr_getpshared();设置：pthread_condattr_setpshared();

// 时钟属性：获取：pthread_condattr_getclock();设置：pthread_condattr_setclock();



**信号量** :

进程的信号量是全局的，有key；线程的信号量是匿名的，没有key

**sem_t** *sem; **sem_init**(); sem_destroy(); 

信号量的P操作：**sem_wait**(); sem_trywait(); sem_timedwait(); 信号量的V操作：sem_post();

获取信号量的值：sem_getvalue();



### 生产消费者模型

基本概念

互斥锁加条件变量实现生产消费者模型（多线程）

pthread_cond_wait(&cond, &mutex);1.互斥锁解锁；2.阻塞，等待被唤醒3.条件触发给互斥锁加锁（原子操作）

信号量实现生产消费者模型（多进程、多线程）

**在pthread_cond_wait时执行pthread_cancel后，要先在线程清理函数中先解锁已与之相应条件绑定的mutex，这样是为了保证pthread_cond_wait可以返回到调用线程**

## 通用模块

/etc/rc.local 中增加开机启动程序脚本  用chmod +x /etc/rc.d/rc.local

/project/tools1/bin/procctl 30 /project/tools1/bin/checkproc

su -wuhm -c "/bin/sh /project /project/idc1/c/start.sh"

### 1. 服务程序的调度模块

模块功能：周期性启动服务程序或shell脚本运行在后台

模块位置：/project/tools1/c/procctl.cpp

模块实现：1.生成fork()子进程，父进程退出 2. while循环中再fork一个进程，子进程使用**execv(argv[2], pargv);**执行程序后退出，父进程调用wait()函数后sleep(atoi(argv[1])) 3.继续重复循环fork



### 2. 检查后台服务程序模块

模块功能：用于检查后台服务程序是否超时，如果已经超时，就终止它

模块位置：/project/tools1/c/checkproc.cpp

模块实现：1.采用shmid = **shmget**((key_t)SHMKEYP, MAXNUMP * sizeof(struct st_procinfo), 0666|IPC_CREAT)

获取位于SHMKEYP的**共享内存**，共享内存中存放着MAXNUMP 个结构体st_procinfo

2.将共享内存连接到当前进程的地址空间struct st_procinfo* shm = (struct st_procinfo*) **shmat**(shmid, 0, 0);

3.遍历共享内存中全部记录，向进程发送信号0，判断它是否还存在，如果不存在，从共享内存中删除该记录int iret = kill(shm[ii].pid, 0);（iret == -1表示不存在）

4.获取当前时间time_t now = time(0);  判断进程是否超时 if(now - shm[ii].atime < shm[ii].timeout)，超时的程序先后使用信号15和信号9停止程序

5.把共享内存从当前进程中分离**shmdt**(shm);

**注：**进程心跳信息的结构体：struct st_procinfo{int    pid;         // 进程id。char   pname[51];   // 进程名称，可以为空。int    timeout;     // 超时时间，单位：秒。 time_t atime;       // 最后一次心跳的时间，用整数表示。};



### 3. 压缩文件模块

模块功能：用于压缩历史的数据文件和日志，把目录及子目录中timeout天之间的匹配matchstr文件全部压缩

模块位置：/project/tools1/c/gzipfiles.cpp

模块实现：1.利用**LocalTime** 函数，获取文件超时的时间点存入字符串strTimeOut

2. 利用**CDir**类**OpenDir（）**打开匹配的文件的**ReadDir**（）函数遍历目录和子目录中的文件

3. 超时时间点比较，如果更早，就需要压缩  即：strcmp(**Dir.m_ModifyTime**, strTimeOut）>=0

4. 利用**MatchStr**(**Dir.m_FileName**, "*.gz")函数过滤已经压缩的文件

5. 调用操作系统的gzip命令

   构造strCmd命令语句**SNPRINTF**(strCmd, sizeof(strCmd), 1000, "/usr/bin/gzip -f %s 1>/dev/null 2>/dev/null", Dir.m_FullFileName);//1>/dev/null 2>/dev/null表示把标准输出和标准输入指向空

   使用**system**(strCmd)执行


### 4. 清理历史数据文件模块

模块功能：删除历史的数据文件和日志，把目录及子目录中timeout天之间的匹配matchstr文件全部删除

模块位置：/project/tools1/c/deletefiles.cpp

模块实现：1.利用**LocalTime** 函数，获取文件超时的时间点存入字符串strTimeOut

2. 利用**CDir**类**OpenDir（）**打开匹配的文件的**ReadDir**（）函数遍历目录和子目录中的文件
3. 超时时间点比较，如果更早，就需要删除  即：strcmp(**Dir.m_ModifyTime**, strTimeOut）>=0
4. 删除文件调用**REMOVE**(Dir.m_FullFileName)

