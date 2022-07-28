/*
    execsql.cpp本程序用于执行一个sql脚本文件
    作者：吴惠明
*/
#include "_public.h"
#include "_mysql.h"

CLogFile logfile;

connection conn;

CPActive PActive;

void EXIT(int sig);

int main(int argc, char* argv[]){
    // 帮助文档
    if(argc != 5){
        printf("\n");
        printf("Using:./execsql sqlfile connstr charset logfile\n");

        printf("Example:/project/tools1/bin/procctl 120 /project/tools1/bin/execsql /project/idc1/sql/cleardata.sql \"127.0.0.1,root,whmhhh1998818,mysql,3306\" utf8 /log/idc/execsql.log\n\n");
        printf("这是一个工具程序，用于执行一个sql脚本文件。\n");
        printf("sqlfile sql脚本文件名，每条sql语句可以多行书写，分号表示一条sql语句的结束，不支持注释。\n");
        printf("connstr 数据库连接参数：ip, username, password, dbname, port\n");
        printf("charset 数据库字符集。\n");
        printf("logfile 本程序运行的日志文件。\n");
        printf("程序每120秒运行一次，由procctl调度。\n\n\n");

        return -1;
    }

    // 处理程序退出的信号
    CloseIOAndSignal(); signal(SIGINT, EXIT); signal(SIGTERM, EXIT);

    // 打开日志文件
    if(logfile.Open(argv[4], "a+") == false){
        printf("打开日志文件是失败(%s)\n", argv[4]);
        return -1;
    }

    PActive.AddPInfo(500, "execsql"); // 进程的心跳
    // 注意，在调试程序的时候，可以启用类似以下的代码，防止超时
    // PActive.AddPInfo(5000, "obtcodetodb"); // 进程的心跳


    // 连接数据库, 不启用事务
    if(conn.connecttodb(argv[2], argv[3], 1) != 0){
        logfile.Write("connect database(%s) failed.\n%s\n", argv[2], conn.m_cda.message);
        return -1;
    }

    logfile.Write("connect database(%s) ok.\n", argv[2]);

    CFile File;
    // 打开SQL文件
    if(File.Open(argv[1], "r") == false){
        logfile.Write("File.Open(%s) failed.\n", argv[1]);
        EXIT(-1);
    }

    char strsql[1001];
    while (true){
        memset(strsql, 0, sizeof(strsql));

        // 从SQL文件中读取以分号结尾的一行
        if(File.FFGETS(strsql, 1000, ";") == false) break;

        // 删除SQL语句最后的分号
        char* pp = strstr(strsql, ";");
        if(pp == 0) continue;
        pp[0] = 0;

        logfile.Write("%s\n", strsql);

        int iret = conn.execute(strsql); // 执行sql语句

        // 把SQL语句执行的结果写入日志
        if(iret == 0) logfile.Write("exec ok(rpc = %d).\n", conn.m_cda.rpc);
        else logfile.Write("exec failed(%s).\n", conn.m_cda.message);

        PActive.UptATime(); // 进程心跳
    }
    logfile.WriteEx("\n");
    return 0;
}

void EXIT(int sig){
    printf("程序退出，sig = %d\n\n", sig);

    conn.disconnect();

    exit(0);
}