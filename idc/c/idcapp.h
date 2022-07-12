/****************************************************************************************/
/*   程序名：idcapp.h，此程序是数据中心项目公用函数和类的声明文件。                     */
/*   作者：吴从周                                                                       */
/****************************************************************************************/

#ifndef IDCAPP_H
#define IDCAPP_H

#include "_public.h"
#include "_mysql.h"

struct st_zhobtmind
{
  char obtid[11];      // 站点代码。
  char ddatetime[21];  // 数据时间，精确到分钟。
  char t[11];          // 温度，单位：0.1摄氏度。
  char p[11];          // 气压，单位：0.1百帕。
  char u[11];          // 相对湿度，0-100之间的值。
  char wd[11];         // 风向，0-360之间的值。
  char wf[11];         // 风速：单位0.1m/s。
  char r[11];          // 降雨量：0.1mm。
  char vis[11];        // 能见度：0.1米。
};

// 全国站点分钟观测数据操作类。
class CZHOBTMIND
{
public:
  connection  *m_conn;     // 数据库连接。
  CLogFile    *m_logfile;  // 日志。

  sqlstatement m_stmt;     // 插入表操作的sql。

  char m_buffer[1024];   // 从文件中读到的一行。
  struct st_zhobtmind m_zhobtmind; // 全国站点分钟观测数据结构。

  CZHOBTMIND();
  CZHOBTMIND(connection *conn,CLogFile *logfile);

 ~CZHOBTMIND();

  void BindConnLog(connection *conn,CLogFile *logfile);  // 把connection和CLogFile的传进去。
  bool SplitBuffer(char *strBuffer,bool bisxml);  // 把从文件读到的一行数据拆分到m_zhobtmind结构体中。
  bool InsertTable();  // 把m_zhobtmind结构体中的数据插入到T_ZHOBTMIND表中。
};



#endif
