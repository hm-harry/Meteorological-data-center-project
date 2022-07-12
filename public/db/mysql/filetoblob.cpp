/*
 *  程序名：filetoblob.cpp，此程序演示开发框架操作MySQL数据库（把图片文件存入BLOB字段）。
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
    long   id;             // 超女编号
    char   pic[100000];    // 超女图片的内容。
    unsigned long picsize; // 图片内容占用的字节数。
  } stgirls;

  sqlstatement stmt(&conn);  // 操作SQL语句的对象。

  // 准备修改表的SQL语句。
  stmt.prepare("update girls set pic=:1 where id=:2");
  stmt.bindinlob(1, stgirls.pic,&stgirls.picsize);
  stmt.bindin(2,&stgirls.id);

  // 修改超女信息表中id为1、2的记录。
  for (int ii=1;ii<3;ii++)
  {
    memset(&stgirls,0,sizeof(struct st_girls));         // 结构体变量初始化。

    // 为结构体变量的成员赋值。
    stgirls.id=ii;                                   // 超女编号。
    // 把图片的内容加载到stgirls.pic中。
    if (ii==1) stgirls.picsize=filetobuf("1.jpg",stgirls.pic);
    if (ii==2) stgirls.picsize=filetobuf("2.jpg",stgirls.pic);

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

