/*
 *  程序名：deletetable.cpp，此程序演示开发框架操作MySQL数据库（删除表中的记录）。
 *  作者：吴从周。
*/

#include "_mysql.h"       // 开发框架操作MySQL的头文件。

int main(int argc,char *argv[])
{
  connection conn;   // 数据库连接类。

  // 登录数据库，返回值：0-成功；其它是失败，存放了MySQL的错误代码。
  // 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
  if (conn.connecttodb("127.0.0.1,root,mysqlpwd,mysql,3306","utf8")!=0)
  {
    printf("connect database failed.\n%s\n",conn.m_cda.message); return -1;
  }

  sqlstatement stmt(&conn);  // 操作SQL语句的对象。

  int iminid,imaxid;  // 删除条件最小和最大的id。

  // 准备删除表的SQL语句。
  stmt.prepare("delete from girls where id>=:1 and id<=:2");
  // 为SQL语句绑定输入变量的地址，bindin方法不需要判断返回值。
  stmt.bindin(1,&iminid);
  stmt.bindin(2,&imaxid);

  iminid=1;    // 指定待删除记录的最小id的值。
  imaxid=3;    // 指定待删除记录的最大id的值。

  // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
  // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // 请注意，stmt.m_cda.rpc变量非常重要，它保存了SQL被执行后影响的记录数。
  printf("本次删除了girls表%ld条记录。\n",stmt.m_cda.rpc);

  conn.commit();   // 提交数据库事务。

  return 0;
}

