/*
 *  程序名：selecttable.cpp，此程序演示开发框架操作PostgreSQL数据库（查询表中的记录）。
 *  作者：吴从周。
*/
#include "_postgresql.h"   // 开发框架操作PostgreSQL的头文件。

// 定义用于超女信息的结构，与表中的字段对应。
struct st_girls
{
  long id;        // 超女编号，用long数据类型对应Oracle无小数的number(10)。
  char name[31];  // 超女姓名，用char[31]对应Oracle的varchar2(30)。
  double weight;  // 超女体重，用double数据类型对应Oracle有小数的number(8,2)。
  char btime[20]; // 报名时间，用char对应Oracle的date，格式：'yyyy-mm-dd hh24:mi:ss'。
} stgirls;

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

  int iminid,imaxid;  // 查询条件最小和最大的id。

  // 准备查询表的SQL语句。
  stmt.prepare("\
    select id,name,weight,to_char(btime,'yyyy-mm-dd hh24:mi:ss') from girls where id>=:1 and id<=:2");
  // prepare方法不需要判断返回值。
  // 为SQL语句绑定输入变量的地址，bindin方法不需要判断返回值。
  stmt.bindin(1,&iminid);
  stmt.bindin(2,&imaxid);
  // 为SQL语句绑定输出变量的地址，bindout方法不需要判断返回值。
  stmt.bindout(1,&stgirls.id);
  stmt.bindout(2, stgirls.name,30);
  stmt.bindout(3,&stgirls.weight);
  stmt.bindout(4, stgirls.btime,19);

  iminid=2;  // 指定待查询记录的最小id的值。
  imaxid=4;  // 指定待查询记录的最大id的值。

  // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
  // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // 本程序执行的是查询语句，执行stmt.execute()后，将会在数据库的缓冲区中产生一个结果集。
  while (1)
  {
    memset(&stgirls,0,sizeof(stgirls)); // 先把结构体变量初始化。

    // 从结果集中获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败。
    // 在实际开发中，除了0和1403，其它的情况极少出现。
    if (stmt.next() !=0) break;
    
    // 把获取到的记录的值打印出来。
    printf("id=%ld,name=%s,weight=%.02f,btime=%s\n",stgirls.id,stgirls.name,stgirls.weight,stgirls.btime);
  }

  // 请注意，stmt.m_cda.rpc变量非常重要，它保存了SQL被执行后影响的记录数。
  printf("本次查询了girls表%ld条记录。\n",stmt.m_cda.rpc);
}

