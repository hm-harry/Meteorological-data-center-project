/*
 *  程序名：deletetable.cpp，此程序演示开发框架操作PostgreSQL数据库（删除表中的记录）。
 *  作者：吴从周。
*/
#include "_postgresql.h"   // 开发框架操作PostgreSQL的头文件。

int main(int argc,char *argv[])
{
  connection conn; // 数据库连接类

  // 登录数据库，返回值：0-成功，其它-失败。
  // 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
  if (conn.connecttodb("host=172.16.0.15 user=postgres password=pwdidc dbname=postgres port=5432","gbk")!=0)
  {
    printf("connect database failed.\n%s\n",conn.m_cda.message); return -1;
  }

  sqlstatement stmt(&conn); // 操作SQL语句的对象。

  // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
  // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
  // 如果不需要绑定输入和输出变量，用stmt.execute()方法直接执行SQL语句，不需要stmt.prepare()。
  if (stmt.execute("delete from girls where id>=2 and id<=4") != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // 请注意，stmt.m_cda.rpc变量非常重要，它保存了SQL被执行后影响的记录数。
  printf("本次从girls表中删除了%ld条记录。\n",stmt.m_cda.rpc); 

  // 提交事务
  conn.commit();
}

