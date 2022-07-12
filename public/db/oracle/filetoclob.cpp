/*
 *  程序名：filetoclob.cpp，此程序演示开发框架操作Oracle数据库。
 *  把当前目录中的memo_in.txt文件写入Oracle的CLOB字段中。
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

  // 为了方便演示，把girls表中的记录全删掉，再插入一条用于测试的数据。
  // 不需要判断返回值
  stmt.prepare("\
    BEGIN\
      delete from girls;\
      insert into girls(id,name,memo) values(1,'超女姓名',empty_clob());\
    END;");
  
  // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // 使用游标从girls表中提取id为1的memo字段
  // 注意了，同一个sqlstatement可以多次使用
  // 但是，如果它的sql改变了，就要重新prepare和bindin或bindout变量
  stmt.prepare("select memo from girls where id=1 for update");
  stmt.bindclob();

  // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }
  
  // 获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败。
  if (stmt.next() != 0) return 0;
  
  // 把磁盘文件memo_in.txt的内容写入CLOB字段
  if (stmt.filetolob((char *)"memo_in.txt") != 0)
  {
    printf("stmt.filetolob() failed.\n%s\n",stmt.m_cda.message); return -1;
  }

  conn.commit(); // 提交事务
}

