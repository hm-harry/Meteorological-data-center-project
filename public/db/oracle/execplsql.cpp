/*
 *  程序名：execplsql.cpp，此程序演示开发框架操作Oracle数据库（执行PL/SQL过程）。
 *  作者：吴从周。
 *  说说我个人的看法，我从不在Oracle数据库中创建PL/SQL过程，也很少使用触发器，原因如下：
 *  1、在Oracle数据库中创建PL/SQL过程，程序的调试很麻烦；
 *  2、维护工作很麻烦，因为维护人员要花时间去了解数据库中的存储过程；
 *  3、采用本开发框架操作Oracle已经是非常简单，没必要去折腾存储过程；
 *  4、PL/SQL过程可移植性不好，如果换成mysql或其它数据库，比较麻烦。
 *  还有，我在C/C++程序中很少用复杂的PL/SQL过程，因为复杂的PL/SQL调试麻烦。
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

  int id=100;

  // 准备查询表的PL/SQL语句，先删除girls表中的全部记录，再插入一条记录。
  stmt.prepare("\
    BEGIN\
      delete from girls;\
      insert into girls(id,name,weight,btime)\
                 values(:1,'超女过程',55.65,to_date('2018-01-02 13:00:55','yyyy-mm-dd hh24:mi:ss'));\
    END;");
  // 注意，PL/SQL中的每条SQL需要用分号结束，END之后还有一个分号。
  // prepare方法不需要判断返回值。
  // 为PL/SQL语句绑定输入变量的地址，bindin方法不需要判断返回值。
  stmt.bindin(1,&id);
  
  // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  printf("exec PL/SQL ok.\n");

  // 提交事务。
  conn.commit();
}

