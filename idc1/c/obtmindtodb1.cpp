/*
    obtmindtodb.cpp本程序用于把全国站点分钟观测数据入库到T_ZHOBTMIND表中，支持xml和csv两种格式
    作者：吴惠明
*/
#include "_public.h"
#include "_mysql.h"

CLogFile logfile;
CFile File;
connection conn;
CPActive PActive;

void EXIT(int sig);

// 业务处理主函数
bool _obtmindtodb(char* pathname, char* connstr, char* charset);

int main(int argc, char* argv[]){
    // 帮助文档
    if(argc != 5){
        printf("\n");
        printf("Using:./obtmindtodb pathname connstr charset logfile\n");

        printf("Example:/project/tools1/bin/procctl 10 /project/idc1/bin/obtmindtodb /idcdata/surfdata \"127.0.0.1,root,whmhhh1998818,mysql,3306\" utf8 /log/idc/obtmindtodb.log\n\n");
        printf("本程序用于把全国站点分钟观测数据入库到T_ZHOBTMIND表中,数据只写入，不更新。\n");
        printf("pathname 全国站点分钟观测数据文件存放目录。\n");
        printf("connstr 数据库连接参数：ip, username, password, dbname, port\n");
        printf("charset 数据库字符集。\n");
        printf("logfile 本程序运行的日志文件。\n");
        printf("程序每10秒运行一次，由procctl调度。\n\n\n");

        return -1;
    }

    // 处理程序退出的信号
    CloseIOAndSignal(); signal(SIGINT, EXIT); signal(SIGTERM, EXIT);

    // 打开日志文件
    if(logfile.Open(argv[4], "a+") == false){
        printf("打开日志文件是失败(%s)\n", argv[4]);
        return -1;
    }

    // PActive.AddPInfo(30, "obtmindtodb"); // 进程的心跳
    // 注意，在调试程序的时候，可以启用类似以下的代码，防止超时
    PActive.AddPInfo(5000, "obtmindtodb"); // 进程的心跳

    _obtmindtodb(argv[1], argv[2], argv[3]);

    // 连接数据库
    // if(conn.connecttodb(argv[2], argv[3]) != 0){
    //     logfile.Write("connect database(%s) failed.\n%s\n", argv[2], conn.m_cda.message);
    //     return -1;
    // }

    // logfile.Write("connect database(%s) ok.\n", argv[2]);

    // // 提交事务
    // conn.commit();

    return 0;
}

void EXIT(int sig){
    printf("程序退出，sig = %d\n\n", sig);

    conn.disconnect();

    exit(0);
}

// 业务处理主函数
bool _obtmindtodb(char* pathname, char* connstr, char* charset){
    CDir Dir;
    // 打开目录
    if(Dir.OpenDir(pathname, "*.xml") == false){
        logfile.Write("Dir.OpenDir(%s) failed.\n", pathname);
        return false;
    }

    while(true){
        // 读取目录，得到一个数据文件名
        if(Dir.ReadDir() == false) break;

        logfile.Write("filename = %s.\n", Dir.m_FullFileName);

    }
    return true;
}