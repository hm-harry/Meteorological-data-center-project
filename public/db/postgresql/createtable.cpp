/*
 *  程序名：createtable.cpp，此程序演示开发框架操作PostgreSQL数据库（创建表）。
 *  作者：吴从周。
*/
#include "_postgresql.h"   // 开发框架操作PostgreSQL的头文件。

int main(int argc,char *argv[])
{
  connection conn; // 数据库连接池。

  // 登录数据库，返回值：0-成功，其它-失败。
  // 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
  if (conn.connecttodb("host=172.16.0.15 user=postgres password=pwdidc dbname=postgres port=5432","gbk")!=0)
  {
    printf("connect database failed.\n%s\n",conn.m_cda.message); return -1;
  }  

  sqlstatement stmt(&conn); // 操作SQL语句的对象。

  // 准备创建表的SQL语句。
  // 超女表girls，超女编号id，超女姓名name，体重weight，报名时间btime，超女说明memo，超女图片pic。
  stmt.prepare("\
    create table girls(id    int,\
                       name  varchar(30),\
                       weight   numeric(8,2),\
                       btime timestamp,\
                       memo  text,\
                       pic   bytea,\
                       primary key (id))");
  // prepare方法不需要判断返回值。

  // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
  // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // 注意，在postgresql数据库中，创建表也要提交事务，和Oracle、MySQL数据库不同。
  conn.commit();

  printf("create table girls ok.\n");
}

