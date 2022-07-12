#ifndef _TOOLS_ORACLE_H
#define _TOOLS_ORACLE_H

#include "_public.h"
#include "_ooci.h"

// 表的列(字段)信息的结构体。
struct st_columns
{
  char  colname[31];  // 列名。
  char  datatype[31]; // 列的数据类型，分为number、date和char三大类。
  int   collen;       // 列的长度，number固定20，date固定19，char的长度由表结构决定。
  int   pkseq;        // 如果列是主键的字段，存放主键字段的顺序，从1开始，不是主键取值0。
};

// 获取表全部的列和主键列信息的类。
class CTABCOLS
{
public:
  CTABCOLS();

  int m_allcount;   // 全部字段的个数。
  int m_pkcount;    // 主键字段的个数。
  int m_maxcollen;  // 全部列中最大的长度，这个成员是后来增加的，课程中并未提及。

  vector<struct st_columns> m_vallcols;  // 存放全部字段信息的容器。
  vector<struct st_columns> m_vpkcols;   // 存放主键字段信息的容器。

  char m_allcols[3001];  // 全部的字段名列表，以字符串存放，中间用半角的逗号分隔。
  char m_pkcols[301];    // 主键字段名列表，以字符串存放，中间用半角的逗号分隔。

  void initdata();  // 成员变量初始化。

  // 获取指定表的全部字段信息。
  bool allcols(connection *conn,char *tablename);

  // 获取指定表的主键字段信息。
  bool pkcols(connection *conn,char *tablename);
};


#endif
