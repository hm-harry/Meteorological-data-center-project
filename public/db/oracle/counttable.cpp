/*
 *  程序名：counttable.cpp，此程序演示开发框架操作Oracle数据库（查询表中的记录数）。
 *  作者：吴从周。
*/
#include "_ooci.h"   // 开发框架操作Oracle的头文件。

int main(int argc,char *argv[])
{
  connection conn; // 数据库连接类
  
  // 登录数据库，返回值：0-成功，其它-失败。
  // 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
  if (conn.connecttodb("scott/tiger@snorcl11g_132","Simplified Chinese_China.AL32UTF8")!=0)
  {
    printf("connect database failed.\n%s\n",conn.m_cda.message); return -1;
  }

  sqlstatement stmt(&conn); // 操作SQL语句的对象。

  int icount=0;  // 用于存放查询结果的记录数。

  // 准备查询表的SQL语句，把查询条件直接写在SQL语句中，没有采用绑定输入变量的方法。
  stmt.prepare("select count(*) from girls where id>=2 and id<=4");
  // prepare方法不需要判断返回值。
  // 为SQL语句绑定输出变量的地址，bindout方法不需要判断返回值。
  stmt.bindout(1,&icount);

  // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
  // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // 本程序执行的是查询语句，执行stmt.execute()后，将会在数据库的缓冲区中产生一个结果集。
  // 但是，在本程序中，结果集永远只有一条记录，调用stmt.next()一次就行，不需要循环。
  stmt.next();
  
  printf("girls表中符合条件的记录数是%d。\n",icount);
}

