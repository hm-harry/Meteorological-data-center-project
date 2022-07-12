/**************************************************************************************/
/*   程序名：_mysql.h，此程序是开发框架的C/C++操作MySQL数据库的声明文件。             */
/*   作者：吴从周。                                                                   */
/**************************************************************************************/

#ifndef __MYSQL_H
#define __MYSQL_H

// C/C++库常用头文件
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include <mysql.h>   // MySQL数据库接口函数的头文件

// 把文件filename加载到buffer中，必须确保buffer足够大。
// 成功返回文件的大小，文件不存在或为空返回0。
unsigned long filetobuf(const char *filename,char *buffer);

// 把buffer中的内容写入文件filename，size为buffer中有效内容的大小。
// 成功返回true，失败返回false。
bool buftofile(const char *filename,char *buffer,unsigned long size);

// MySQL登录环境
struct LOGINENV
{
  char ip[32];       // MySQL数据库的ip地址。
  int  port;         // MySQL数据库的通信端口。
  char user[32];     // 登录MySQL数据库的用户名。
  char pass[32];     // 登录MySQL数据库的密码。
  char dbname[51];   // 登录后，缺省打开的数据库。
};

struct CDA_DEF         // 每次调用MySQL接口函数返回的结果。
{
  int      rc;         // 返回值：0-成功；其它是失败，存放了MySQL的错误代码。
  unsigned long rpc;   // 如果是insert、update和delete，存放影响记录的行数，如果是select，存放结果集的行数。
  char     message[2048]; // 如果返回失败，存放错误描述信息。
};

// MySQL数据库连接类。
class connection
{
private:
  // 从connstr中解析ip,username,password,dbname,port。
  void setdbopt(const char *connstr);

  // 设置字符集，要与数据库的一致，否则中文会出现乱码。
  void character(const char *charset);

  LOGINENV m_env;      // 服务器环境句柄。

  char m_dbtype[21];   // 数据库种类，固定取值为"mysql"。
public:
  int m_state;         // 与数据库的连接状态，0-未连接，1-已连接。

  CDA_DEF m_cda;       // 数据库操作的结果或最后一次执行SQL语句的结果。

  char m_sql[10241];   // SQL语句的文本，最长不能超过10240字节。

  connection();        // 构造函数。
 ~connection();        // 析构函数。

  // 登录数据库。
  // connstr：数据库的登录参数，格式："ip,username,password,dbname,port"，
  // 例如："172.16.0.15,qxidc,qxidcpwd,qxidcdb,3306"。
  // charset：数据库的字符集，如"utf8"、"gbk"，必须与数据库保持一致，否则会出现中文乱码的情况。
  // autocommitopt：是否启用自动提交，0-不启用，1-启用，缺省是不启用。
  // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
  int connecttodb(const char *connstr,const char *charset,unsigned int autocommitopt=0);

  // 提交事务。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  int commit();

  // 回滚事务。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  int  rollback();

  // 断开与数据库的连接。
  // 注意，断开与数据库的连接时，全部未提交的事务自动回滚。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  int disconnect();

  // 执行SQL语句。
  // 如果SQL语句不需要绑定输入和输出变量（无绑定变量、非查询语句），可以直接用此方法执行。
  // 参数说明：这是一个可变参数，用法与printf函数相同。
  // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中，
  // 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
  // 程序中必须检查execute方法的返回值。
  // 在connection类中提供了execute方法，是为了方便程序员，在该方法中，也是用sqlstatement类来完成功能。
  int execute(const char *fmt,...);

  ////////////////////////////////////////////////////////////////////
  // 以下成员变量和函数，除了sqlstatement类，在类的外部不需要调用它。
  MYSQL     *m_conn;   // MySQL数据库连接句柄。
  int m_autocommitopt; // 自动提交标志，0-关闭自动提交；1-开启自动提交。
  void err_report();   // 获取错误信息。
  ////////////////////////////////////////////////////////////////////
};

// 执行SQL语句前绑定输入或输出变量个数的最大值，256是很大的了，可以根据实际情况调整。
#define MAXPARAMS  256

// 操作SQL语句类。
class sqlstatement
{
private:
  MYSQL_STMT *m_handle; // SQL语句句柄。
  
  MYSQL_BIND params_in[MAXPARAMS];            // 输入参数。
  unsigned long params_in_length[MAXPARAMS];  // 输入参数的实际长度。
  my_bool params_in_is_null[MAXPARAMS];       // 输入参数是否为空。
  unsigned maxbindin;                         // 输入参数最大的编号。

  MYSQL_BIND params_out[MAXPARAMS]; // 输出参数。

  CDA_DEF m_cda1;      // prepare() SQL语句的结果。
  
  connection *m_conn;  // 数据库连接指针。
  int m_sqltype;       // SQL语句的类型，0-查询语句；1-非查询语句。
  int m_autocommitopt; // 自动提交标志，0-关闭；1-开启。
  void err_report();   // 错误报告。
  void initial();      // 初始化成员变量。
public:
  int m_state;         // 与数据库连接的绑定状态，0-未绑定，1-已绑定。

  char m_sql[10241];   // SQL语句的文本，最长不能超过10240字节。

  CDA_DEF m_cda;       // 执行SQL语句的结果。

  sqlstatement();      // 构造函数。
  sqlstatement(connection *conn);    // 构造函数，同时绑定数据库连接。

 ~sqlstatement();      // 析构函数。

  // 绑定数据库连接。
  // conn：数据库连接connection对象的地址。
  // 返回值：0-成功，其它失败，只要conn参数是有效的，并且数据库的游标资源足够，connect方法不会返回失败。
  // 程序中一般不必关心connect方法的返回值。
  // 注意，每个sqlstatement只需要绑定一次，在绑定新的connection前，必须先调用disconnect方法。
  int connect(connection *conn);

  // 取消与数据库连接的绑定。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  int disconnect();

  // 准备SQL语句。
  // 参数说明：这是一个可变参数，用法与printf函数相同。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  // 注意：如果SQL语句没有改变，只需要prepare一次就可以了。
  int prepare(const char *fmt,...);

  // 绑定输入变量的地址。
  // position：字段的顺序，从1开始，必须与prepare方法中的SQL的序号一一对应。
  // value：输入变量的地址，如果是字符串，内存大小应该是表对应的字段长度加1。
  // len：如果输入变量的数据类型是字符串，用len指定它的最大长度，建议采用表对应的字段长度。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  // 注意：1）如果SQL语句没有改变，只需要bindin一次就可以了，2）绑定输入变量的总数不能超过MAXPARAMS个。
  int bindin(unsigned int position,int    *value);
  int bindin(unsigned int position,long   *value);
  int bindin(unsigned int position,unsigned int  *value);
  int bindin(unsigned int position,unsigned long *value);
  int bindin(unsigned int position,float *value);
  int bindin(unsigned int position,double *value);
  int bindin(unsigned int position,char   *value,unsigned int len);
  // 绑定BLOB字段，buffer为BLOB字段的内容，size为BLOB字段的大小。
  int bindinlob(unsigned int position,void *buffer,unsigned long *size);

  // 把结果集的字段与变量的地址绑定。
  // position：字段的顺序，从1开始，与SQL的结果集字段一一对应。
  // value：输出变量的地址，如果是字符串，内存大小应该是表对应的字段长度加1。
  // len：如果输出变量的数据类型是字符串，用len指定它的最大长度，建议采用表对应的字段长度。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  // 注意：1）如果SQL语句没有改变，只需要bindout一次就可以了，2）绑定输出变量的总数不能超过MAXPARAMS个。
  int bindout(unsigned int position,int    *value);
  int bindout(unsigned int position,long   *value);
  int bindout(unsigned int position,unsigned int  *value);
  int bindout(unsigned int position,unsigned long *value);
  int bindout(unsigned int position,float *value);
  int bindout(unsigned int position,double *value);
  int bindout(unsigned int position,char   *value,unsigned int len);
  // 绑定BLOB字段，buffer用于存放BLOB字段的内容，buffersize为buffer占用内存的大小，
  // size为结果集中BLOB字段实际的大小，注意，一定要保证buffer足够大，防止内存溢出。
  int bindoutlob(unsigned int position,void *buffer,unsigned long buffersize,unsigned long *size);

  // 执行SQL语句。
  // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
  // 如果成功的执行了insert、update和delete语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
  // 程序中必须检查execute方法的返回值。
  int execute();

  // 执行SQL语句。
  // 如果SQL语句不需要绑定输入和输出变量（无绑定变量、非查询语句），可以直接用此方法执行。
  // 参数说明：这是一个可变参数，用法与printf函数相同。
  // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中，
  // 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
  // 程序中必须检查execute方法的返回值。
  int execute(const char *fmt,...);

  // 从结果集中获取一条记录。
  // 如果执行的SQL语句是查询语句，调用execute方法后，会产生一个结果集（存放在数据库的缓冲区中）。
  // next方法从结果集中获取一条记录，把字段的值放入已绑定的输出变量中。
  // 返回值：0-成功，1403-结果集已无记录，其它-失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
  // 返回失败的原因主要有两种：1）与数据库的连接已断开；2）绑定输出变量的内存太小。
  // 每执行一次next方法，m_cda.rpc的值加1。
  // 程序中必须检查next方法的返回值。
  int next();
};

#endif

