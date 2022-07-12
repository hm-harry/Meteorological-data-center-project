#include "_tools_oracle.h"

// 获取表全部的列和主键列信息的类。
CTABCOLS::CTABCOLS()
{
  initdata();  // 调用成员变量初始化函数。
}

void CTABCOLS::initdata()  // 成员变量初始化。
{
  m_allcount=m_pkcount=0;
  m_maxcollen=0;
  m_vallcols.clear();
  m_vpkcols.clear();
  memset(m_allcols,0,sizeof(m_allcols));
  memset(m_pkcols,0,sizeof(m_pkcols));
}

// 获取指定表的全部字段信息。
bool CTABCOLS::allcols(connection *conn,char *tablename)
{
  m_allcount=0;
  m_maxcollen=0;
  m_vallcols.clear();
  memset(m_allcols,0,sizeof(m_allcols));

  struct st_columns stcolumns;

  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select lower(column_name),lower(data_type),data_length from USER_TAB_COLUMNS where table_name=upper(:1) order by column_id");
  stmt.bindin(1,tablename,30);
  stmt.bindout(1, stcolumns.colname,30);
  stmt.bindout(2, stcolumns.datatype,30);
  stmt.bindout(3,&stcolumns.collen);

  if (stmt.execute()!=0) return false;

  while (true)
  {
    memset(&stcolumns,0,sizeof(struct st_columns));
  
    if (stmt.next()!=0) break;

    // 列的数据类型，分为number、date和char三大类。
    if (strcmp(stcolumns.datatype,"char")==0)     strcpy(stcolumns.datatype,"char");
    if (strcmp(stcolumns.datatype,"varchar")==0)  strcpy(stcolumns.datatype,"char");
    if (strcmp(stcolumns.datatype,"varchar2")==0) strcpy(stcolumns.datatype,"char");

    if (strcmp(stcolumns.datatype,"datetime")==0)  strcpy(stcolumns.datatype,"date");
    if (strcmp(stcolumns.datatype,"timestamp")==0) strcpy(stcolumns.datatype,"date");
    
    if (strcmp(stcolumns.datatype,"tinyint")==0)   strcpy(stcolumns.datatype,"number");
    if (strcmp(stcolumns.datatype,"smallint")==0)  strcpy(stcolumns.datatype,"number");
    if (strcmp(stcolumns.datatype,"mediumint")==0) strcpy(stcolumns.datatype,"number");
    if (strcmp(stcolumns.datatype,"int")==0)       strcpy(stcolumns.datatype,"number");
    if (strcmp(stcolumns.datatype,"integer")==0)   strcpy(stcolumns.datatype,"number");
    if (strcmp(stcolumns.datatype,"bigint")==0)    strcpy(stcolumns.datatype,"number");
    if (strcmp(stcolumns.datatype,"numeric")==0)   strcpy(stcolumns.datatype,"number");
    if (strcmp(stcolumns.datatype,"decimal")==0)   strcpy(stcolumns.datatype,"number");
    if (strcmp(stcolumns.datatype,"float")==0)     strcpy(stcolumns.datatype,"number");
    if (strcmp(stcolumns.datatype,"double")==0)    strcpy(stcolumns.datatype,"number");

    // 如果业务有需要，可以修改上面的代码，增加对更多数据类型的支持。
    // 如果字段的数据类型不在上面列出来的中，忽略它。
    if ( (strcmp(stcolumns.datatype,"char")!=0) &&
         (strcmp(stcolumns.datatype,"date")!=0) &&
         (strcmp(stcolumns.datatype,"number")!=0) ) continue;

    // 如果字段类型是date，把长度设置为19。yyyy-mm-dd hh:mi:ss
    if (strcmp(stcolumns.datatype,"date")==0) stcolumns.collen=19;

    // 如果字段类型是number，把长度设置为20。
    if (strcmp(stcolumns.datatype,"number")==0) stcolumns.collen=20;

    strcat(m_allcols,stcolumns.colname); strcat(m_allcols,",");

    m_vallcols.push_back(stcolumns);

    if (m_maxcollen<stcolumns.collen) m_maxcollen=stcolumns.collen;

    m_allcount++;
  }

  // 删除m_allcols最后一个多余的逗号。
  if (m_allcount>0) m_allcols[strlen(m_allcols)-1]=0;

  return true;
}

// 获取指定表的主键字段信息。
bool CTABCOLS::pkcols(connection *conn,char *tablename)
{
  m_pkcount=0;
  memset(m_pkcols,0,sizeof(m_pkcols));
  m_vpkcols.clear();

  struct st_columns stcolumns;

  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select lower(column_name),position from USER_CONS_COLUMNS\
       where table_name=upper(:1)\
         and constraint_name=(select constraint_name from USER_CONSTRAINTS\
                               where table_name=upper(:2) and constraint_type='P'\
                                 and generated='USER NAME')\
       order by position");
  stmt.bindin(1,tablename,30);
  stmt.bindin(2,tablename,30);
  stmt.bindout(1, stcolumns.colname,30);
  stmt.bindout(2,&stcolumns.pkseq);

  if (stmt.execute() != 0) return false;

  while (true)
  {
    memset(&stcolumns,0,sizeof(struct st_columns));

    if (stmt.next() != 0) break;

    strcat(m_pkcols,stcolumns.colname); strcat(m_pkcols,",");

    m_vpkcols.push_back(stcolumns);

    m_pkcount++;
  }

  if (m_pkcount>0) m_pkcols[strlen(m_pkcols)-1]=0;    // 删除m_pkcols最后一个多余的逗号。

  return true;
}
