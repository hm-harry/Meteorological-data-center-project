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

struct st_zhobtmind{
    char obtid[11];      // 站点代码
    char ddatetime[21];  // 数据时间：格式yyyymmddhh24miss
    char t[11];              // 气温：单位，0.1摄氏度。
    char p[11];              // 气压：0.1百帕。
    char u[11];              // 相对湿度，0-100之间的值。
    char wd[11];             // 风向，0-360之间的值。
    char wf[11];             // 风速：单位0.1m/s
    char r[11];              // 降雨量：0.1mm。
    char vis[11];            // 能见度：0.1米。
};

// 全国站点分钟观测数据操作类
class CZHOBTMIND{
public:
    connection* m_conn; // 数据库连接
    CLogFile* m_logfile; // 日志
    sqlstatement m_stmt; // 插入表操作的sql
    char m_buffer[1024]; // 从文件中读到一行
    struct st_zhobtmind m_zhobtmind; // 全国站点分钟观测数据结构
    CZHOBTMIND();
    CZHOBTMIND(connection* conn, CLogFile* logfile);
    ~CZHOBTMIND();

    void BindConnLog(connection* conn, CLogFile* logfile);
    bool SplitBuffer(char* strBuffer); // 把文件读到的一行数据拆分到m_zhobtmind结构体中
    bool InsertTable(); // 把m_zhobtmind结构体中的数据插入T_ZHOBTMIND表中
};

CZHOBTMIND::CZHOBTMIND(){
    m_conn = 0;
    m_logfile = 0;
}

CZHOBTMIND::CZHOBTMIND(connection* conn, CLogFile* logfile){
    m_conn = conn;
    m_logfile = logfile;
}

CZHOBTMIND::~CZHOBTMIND(){}

void CZHOBTMIND::BindConnLog(connection* conn, CLogFile* logfile){
    m_conn = conn;
    m_logfile = logfile;
}

// 把文件读到的一行数据拆分到m_zhobtmind结构体中
bool CZHOBTMIND::SplitBuffer(char* strBuffer){
    memset(&m_zhobtmind, 0, sizeof(struct st_zhobtmind));
    GetXMLBuffer(strBuffer, "obtid", m_zhobtmind.obtid, 10);
    GetXMLBuffer(strBuffer, "ddatetime", m_zhobtmind.ddatetime, 14);
    char tmp[11];
    GetXMLBuffer(strBuffer, "t", tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.t, 10, "%d", (int)(atof(tmp)*10));
    GetXMLBuffer(strBuffer, "p", tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.p, 10, "%d", (int)(atof(tmp)*10));
    GetXMLBuffer(strBuffer, "u", m_zhobtmind.u, 10);
    GetXMLBuffer(strBuffer, "wd", m_zhobtmind.wd, 10);
    GetXMLBuffer(strBuffer, "wf", tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.wf, 10, "%d", (int)(atof(tmp)*10));
    GetXMLBuffer(strBuffer, "r", tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.r, 10, "%d", (int)(atof(tmp)*10));
    GetXMLBuffer(strBuffer, "vis", tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.vis, 10, "%d", (int)(atof(tmp)*10));

    STRCPY(m_buffer, sizeof(m_buffer), strBuffer);

    return true;
}

// 把m_zhobtmind结构体中的数据插入T_ZHOBTMIND表中
bool CZHOBTMIND::InsertTable(){
    if(m_stmt.m_state == 0){
        m_stmt.connect(m_conn);
        m_stmt.prepare("insert into T_ZHOBTMIND(obtid, ddatetime, t, p, u, wd, wf, r, vis) values(:1, str_to_date(:2, '%%Y%%m%%d%%H%%i%%s'), :3, :4, :5, :6, :7, :8, :9)");
        m_stmt.bindin(1, m_zhobtmind.obtid, 10);
        m_stmt.bindin(2, m_zhobtmind.ddatetime, 14);
        m_stmt.bindin(3, m_zhobtmind.t, 10);
        m_stmt.bindin(4, m_zhobtmind.p, 10);
        m_stmt.bindin(5, m_zhobtmind.u, 10);
        m_stmt.bindin(6, m_zhobtmind.wd, 10);
        m_stmt.bindin(7, m_zhobtmind.wf, 10);
        m_stmt.bindin(8, m_zhobtmind.r, 10);
        m_stmt.bindin(9, m_zhobtmind.vis, 10);
    }
    // 把结构体中的数据插入表中
    if(m_stmt.execute() != 0){
        if(m_stmt.m_cda.rc != 1062){
            m_logfile->Write("Buffer = %s\n", m_buffer);
            m_logfile->Write("m_stmt.execute() failed. \n%s\n%s\n", m_stmt.m_sql, m_stmt.m_cda.message);
        }
        return false;
    }
    return true;
}

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

    CFile File;

    CZHOBTMIND ZHOBTMIND(&conn, &logfile);

    int totalcount = 0; // 文件总记录数
    int insertcount = 0; // 成功插入记录数
    CTimer Timer; // 计时器，记录每个文件的处理时间

    while(true){
        // 读取目录，得到一个数据文件名
        if(Dir.ReadDir() == false) break;

        // 连接数据库
        if(conn.m_state == 0){
            if(conn.connecttodb(connstr, charset) != 0){
                logfile.Write("connect database(%s) failed.\n%s\n", connstr, conn.m_cda.message);
                return -1;
            }

            logfile.Write("connect database(%s) ok.\n", connstr);
        }

        // 打开文件
        if(File.Open(Dir.m_FullFileName, "r") == false){
            logfile.Write("File.Open(%s) failed.\n", Dir.m_FullFileName);
            return false;
        }

        totalcount = 0;
        insertcount = 0;

        char strBuffer[1001];// 存放从文件中读取的一行
        
        while(true){
            if(File.FFGETS(strBuffer, 1000, "<endl/>") == false) break;

            // 处理文件中的每一行
            ++totalcount;

            ZHOBTMIND.SplitBuffer(strBuffer);

            if(ZHOBTMIND.InsertTable() == true) ++insertcount;
        }
        // 删除文件提交事务
        // File.CloseAndRemove();
        conn.commit();

        logfile.Write("已处理文件%s (totalcount = %d, insertcount = %d), 耗时%.2f秒。\n", Dir.m_FullFileName, totalcount, insertcount, Timer.Elapsed());
    }
    return true;
}