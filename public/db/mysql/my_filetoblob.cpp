/*
 程序名：my_filetoblob.cpp.此程序演示开发框架MySQL数据库（向表中插入图片数据）
 作者：吴惠明
*/

#include "_mysql.h" // 开发框架操作MySQL的头文件

int main(int argc, char* argv[]){
    connection conn; // 数据库连接类

    // 登录数据库，返回值：0-成功，其他是失败，存放了MySQL的错误代码。
    // 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中
    if(conn.connecttodb("127.0.0.1,root,Whmhhh1998818!,mysql,3306", "utf8") != 0){
        printf("connect database failed.\n%s\n", conn.m_cda.message);
        return -1;
    }

    // 定义用于超女信息的结构，与表中字段对应
    struct st_girls{
        long id; // 超女编号id
        char pic[100000]; // 超女图片内容
        unsigned long picsize; // 图片内容占用的字节数
    }stgirls;

    sqlstatement stmt(&conn); // 操作SQL语句的对象

    // 准备插入表的SQL语句
    stmt.prepare("update girls set pic = :1 where id = :2");
    
    stmt.bindinlob(1, stgirls.pic, &stgirls.picsize);
    stmt.bindin(2, &stgirls.id);

    // 模拟超女数据，修改超女信息表中的全部记录
    for(int ii = 1; ii < 3; ++ii){
        memset(&stgirls, 0, sizeof(struct st_girls)); // 结构体变量初始化

        // 为结构体变量的成员赋值
        stgirls.id = ii; // 超女编号id
        // 把图片的内容加载到stgirls.pic中
        if(ii == 1) stgirls.picsize = filetobuf("1.jpg", stgirls.pic);
        if(ii == 2) stgirls.picsize = filetobuf("2.jpg", stgirls.pic);

        if(stmt.execute() != 0){
            printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql, stmt.m_cda.message);
            return -1;
        }

        printf("成功修改了%ld条数据。\n",stmt.m_cda.rpc);
        
    }

    conn.commit(); // 提交数据库事务

    printf("update table girls ok.\n");

    return 0;
}