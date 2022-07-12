/**************************************************************************************/
/*   程序名：_ooci.h，此程序是开发框架的C/C++操作Oracle数据库的声明文件。             */
/*   作者：吴从周。                                                                   */
/**************************************************************************************/

#ifndef __OOCI_H
#define __OOCI_H

// C/C++库常用头文件
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <oci.h>     // OCI的头文件。

struct LOGINENV      // OCI登录环境。
{
  char user[32];     // 数据库的用户名。
  char pass[32];     // 数据库的密码。
  char tnsname[51];  // 数据库的tnsname，在ORACLE_HOME/network/admin/tnsnames.ora中配置。

  OCIEnv *envhp;     // 环境变量的句柄。
};

struct OCI_CXT       // OCI上下文句柄。
{
  OCISvcCtx  *svchp;
  OCIError   *errhp;
  OCIEnv     *envhp; // 环境变量的句柄。
};

struct OCI_HANDLE    // OCI的SQL句柄。
{
  OCISvcCtx  *svchp; // 服务器上下文的句柄引用context句柄。
  OCIStmt    *smthp;

  OCIBind    *bindhp;
  OCIDefine  *defhp;

  OCIError   *errhp; // 错误句柄引用context句柄。

  OCIEnv     *envhp; // 环境变量的句柄。
};

struct CDA_DEF       // OCI接口函数执行的结果。
{
  int      rc;          // 返回值：0-成功，其它失败。
  unsigned long rpc;    // 如果是insert、update和delete，保存影响记录的行数，如果是select，保存结果集的行数。
  char     message[2048];  // 执行SQL语句如果失败，存放错误描述信息。
};

int oci_init(LOGINENV *env);
int oci_close(LOGINENV *env); 
int oci_context_create(LOGINENV *env,OCI_CXT *cxt);
int oci_context_close(OCI_CXT *cxt);

int oci_stmt_create(OCI_CXT *cxt,OCI_HANDLE *handle);
int oci_stmt_close(OCI_HANDLE *handle);

// Oracle数据库连接类。
class connection
{
private:
  LOGINENV m_env;    // 服务器环境句柄。

  char m_dbtype[21]; // 数据库种类，固定取值为"oracle"。

  // 设置字符集，如果客户端的字符集与数据库的不一致，就会出现乱码。
  void character(const char *charset);

  // 从connstr中解析username、password和tnsname。
  void setdbopt(const char *connstr);
public:
  int m_state;       // 与数据库的连接状态，0-未连接，1-已连接。

  CDA_DEF m_cda;       // 数据库操作的结果或最后一次执行SQL语句的结果。

  char m_sql[10241];   // SQL语句的文本，最长不能超过10240字节。

  connection();    // 构造函数。
 ~connection();    // 析构函数。

  // 登录数据库。
  // connstr：数据库的登录参数，格式：username/password@tnsname，username-用户名，password-登录密
  // 码，tnsname-数据库的服务名，在$ORACLE_HOME/network/admin/tnsnames.ora文件中配置。
  // charset：数据库的字符集，必须与数据库保持一致，否则会出现中文乱码的情况。
  // autocommitopt：是否启用自动提交，0-不启用，1-启用，缺省是不启用。
  // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
  int connecttodb(const char *connstr,const char *charset,int autocommitopt=0);

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
  // 在connection类中提供了execute方法，是为了方便程序中，在该方法中，也是用sqlstatement类来完成功能。
  int execute(const char *fmt,...);

  ////////////////////////////////////////////////////////////////////
  // 以下成员变量和函数，除了sqlstatement类，在类的外部不需要调用它。
  int m_autocommitopt; // 自动提交标志，0-关闭；1-开启。
  OCI_CXT m_cxt;       // 服务器上下文。
  void err_report();   // 获取错误信息。
  ////////////////////////////////////////////////////////////////////
};

// 操作SQL语句类。
class sqlstatement
{
  OCI_HANDLE m_handle; // SQL句柄。
  connection *m_conn;  // 数据库连接指针。
  int m_sqltype;       // SQL语句的类型，0-查询语句；1-非查询语句。
  int m_autocommitopt; // 自动提交标志，0-关闭；1-开启。
  void err_report();   // 错误报告。

  OCILobLocator *m_lob;     // 指向LOB字段的指针。
  int  alloclob();          // 初始化lob指针。
  int  filetolob(FILE *fp); // 把文件的内容导入到clob和blob字段中。
  int  lobtofile(FILE *fp); // 从clob和blob字段中导出内容到文件中。
  void freelob();           // 释放lob指针。
public:
  int m_state;         // 与数据库连接的绑定状态，0-未绑定，1-已绑定。

  char m_sql[10241];   // SQL语句的文本，最长不能超过10240字节。

  CDA_DEF m_cda;       // 执行SQL语句的结果。

  sqlstatement();      // 构造函数。
  sqlstatement(connection *conn);    // 构造函数，同时绑定数据库连接。

 ~sqlstatement();

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
  // 注意：如果SQL语句没有改变，只需要bindin一次就可以了。
  int bindin(unsigned int position,int    *value);
  int bindin(unsigned int position,long   *value);
  int bindin(unsigned int position,unsigned int  *value);
  int bindin(unsigned int position,unsigned long *value);
  int bindin(unsigned int position,float *value);
  int bindin(unsigned int position,double *value);
  int bindin(unsigned int position,char   *value,unsigned int len);

  // 绑定输出变量的地址。
  // position：字段的顺序，从1开始，与SQL的结果集一一对应。
  // value：输出变量的地址，如果是字符串，内存大小应该是表对应的字段长度加1。
  // len：如果输出变量的数据类型是字符串，用len指定它的最大长度，建议采用表对应的字段长度。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  // 注意：如果SQL语句没有改变，只需要bindout一次就可以了。
  int bindout(unsigned int position,int    *value);
  int bindout(unsigned int position,long   *value);
  int bindout(unsigned int position,unsigned int  *value);
  int bindout(unsigned int position,unsigned long *value);
  int bindout(unsigned int position,float *value);
  int bindout(unsigned int position,double *value);
  int bindout(unsigned int position,char   *value,unsigned int len);

  // 执行SQL语句。
  // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
  // 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
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
  // 返回失败的原因主要有两个：1）与数据库的连接已断开；2）绑定输出变量的内存太小。
  // 每执行一次next方法，m_cda.rpc的值加1。
  // 程序中必须检查next方法的返回值。
  int next();

  // 绑定clob字段。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  int bindblob();

  // 绑定blob字段。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  int bindclob();

  // 把文件的内容导入到clob和blob字段中。
  // filename：待导入的文件名，建议采用绝对路径。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  int filetolob(char *filename);

  // 从clob和blob字段中导出内容到文件中。
  // filename：导出内容存放的文件名，建议采用绝对路径。
  // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
  int lobtofile(char *filename);
};

#endif 

