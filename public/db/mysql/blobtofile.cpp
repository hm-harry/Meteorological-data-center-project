/*
 *  程序名：blobtofile.cpp，此程序演示开发框架操作MySQL数据库（提取BLOB字段内容到图片文件中）。
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

  // 准备查询表的SQL语句。
  stmt.prepare("select id,pic from girls where id in (1,2)");
  stmt.bindout(1,&stgirls.id);
  stmt.bindoutlob(2, stgirls.pic,100000,&stgirls.picsize);

  // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
  // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
  if (stmt.execute()!=0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // 本程序执行的是查询语句，执行stmt.execute()后，将会在数据库的缓冲区中产生一个结果集。
  while (true)
  {
    memset(&stgirls,0,sizeof(stgirls)); // 先把结构体变量初始化。

    // 从结果集中获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败。
    // 在实际开发中，除了0和1403，其它的情况极少出现。
    if (stmt.next()!=0) break;

    // 生成文件名。
    char filename[101]; memset(filename,0,sizeof(filename));
    sprintf(filename,"%d_out.jpg",stgirls.id);
    
    // 把内容写入文件。
    buftofile(filename,stgirls.pic,stgirls.picsize);
  }

  // 请注意，stmt.m_cda.rpc变量非常重要，它保存了SQL被执行后影响的记录数。
  printf("本次查询了girls表%ld条记录。\n",stmt.m_cda.rpc);

  return 0;
}

