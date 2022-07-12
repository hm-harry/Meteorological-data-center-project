/*
 *  程序名：updatetable.cpp，此程序演示开发框架操作MySQL数据库（修改表中的记录）。
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

  // 定义用于超女信息的结构，与表中的字段对应。
  struct st_girls
  {
    long   id;        // 超女编号
    char   name[31];  // 超女姓名
    double weight;    // 超女体重
    char   btime[20]; // 报名时间
  } stgirls;

  sqlstatement stmt(&conn);  // 操作SQL语句的对象。

  // 准备修改表的SQL语句。
  stmt.prepare("\
    update girls set name=:1,weight=:2,btime=str_to_date(:3,'%%Y-%%m-%%d %%H:%%i:%%s') where id=:4");
  /*
    注意事项：
    1、参数的序号从1开始，连续、递增，参数也可以用问号表示，但是，问号的兼容性不好，不建议；
    2、SQL语句中的右值才能作为参数，表名、字段名、关键字、函数名等都不能作为参数；
    3、参数可以参与运算或用于函数的参数；
    4、如果SQL语句的主体没有改变，只需要prepare()一次就可以了；
    5、SQL语句中的每个参数，必须调用bindin()绑定变量的地址；
    6、如果SQL语句的主体已改变，prepare()后，需重新用bindin()绑定变量；
    7、prepare()方法有返回值，一般不检查，如果SQL语句有问题，调用execute()方法时能发现；
    8、bindin()方法的返回值固定为0，不用判断返回值；
    9、prepare()和bindin()之后，每调用一次execute()，就执行一次SQL语句，SQL语句的数据来自被绑定变量的值。
  */
  stmt.bindin(1, stgirls.name,30);
  stmt.bindin(2,&stgirls.weight);
  stmt.bindin(3, stgirls.btime,19);
  stmt.bindin(4,&stgirls.id);

  // 模拟超女数据，修改超女信息表中的全部记录。
  for (int ii=0;ii<5;ii++)
  {
    memset(&stgirls,0,sizeof(struct st_girls));         // 结构体变量初始化。

    // 为结构体变量的成员赋值。
    stgirls.id=ii+1;                                   // 超女编号。
    sprintf(stgirls.name,"貂蝉%05dgirl",ii+1);         // 超女姓名。
    stgirls.weight=ii+48.39;                           // 超女体重。
    sprintf(stgirls.btime,"2021-10-02 11:25:%02d",ii); // 报名时间。 

    // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
    // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
    if (stmt.execute()!=0)
    {
      printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
    }

    printf("成功修改了%ld条记录。\n",stmt.m_cda.rpc); // stmt.m_cda.rpc是本次执行SQL影响的记录数。
  }

  printf("update table girls ok.\n");

  conn.commit();   // 提交数据库事务。

  return 0;
}

