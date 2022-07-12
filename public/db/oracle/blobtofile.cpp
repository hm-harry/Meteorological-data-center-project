/*
 *  程序名：bolbtofile.cpp，此程序演示开发框架操作Oracle数据库。
 *  把Oracle的BLOB字段的内容提取到目前目录的pic_out.jpeg中。
 *  作者：吴从周。
*/
#include "_ooci.h"   // 开发框架操作Oracle的头文件。

int main(int argc,char *argv[])
{
  connection conn; // 数据库连接类

  // 连接数据库，返回值0-成功，其它-失败
  // 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
  if (conn.connecttodb("scott/tiger@snorcl11g_132","Simplified Chinese_China.AL32UTF8") != 0)
  {
    printf("connect database failed.\n%s\n",conn.m_cda.message); return -1;
  }

  sqlstatement stmt(&conn); // SQL语句操作类

  // 不需要判断返回值
  stmt.prepare("select pic from girls where id=1");
  stmt.bindblob();

  // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // 获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败。
  if (stmt.next() != 0) return 0;

  // 把BLOB字段中的内容写入磁盘文件，一定要判断返回值，0-成功，其它-失败。
  if (stmt.lobtofile((char *)"pic_out.jpeg") != 0)
  {
    printf("stmt.lobtofile() failed.\n%s\n",stmt.m_cda.message); return -1;
  }
}

