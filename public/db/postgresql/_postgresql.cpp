/****************************************************************************************/
/*   程序名：_postgresql.h，此程序是开发框架的C/C++操作postgresql数据库的定义文件。     */
/*   作者：吴从周                                                                       */
/****************************************************************************************/

#include "_postgresql.h"

connection::connection()
{
  m_conn=0;

  m_state = 0;

  memset(&m_cda,0,sizeof(m_cda));

  m_cda.rc=-1;
  strcpy(m_cda.message,"database not open.");

  // 数据库种类
  memset(m_dbtype,0,sizeof(m_dbtype));
  strcpy(m_dbtype,"postgresql");
}

connection::~connection()
{
  disconnect();
}

int connection::connecttodb(char *connstr,char *charset,unsigned int autocommitopt)
{
  // 如果已连接上数据库，就不再连接
  // 所以，如果想重连数据库，必须显示的调用disconnect()方法后才能重连
  if (m_state == 1) return 0;

  m_conn=PQconnectdb(connstr);

  if (PQstatus(m_conn) != CONNECTION_OK)
  {
    m_cda.rc=-1; strcpy(m_cda.message,PQerrorMessage(m_conn)); return -1;
  }

  m_state = 1;

  // 设置字符集
  character(charset);

  m_autocommitopt=autocommitopt;

  begin();

  return 0;
}

// 设置字符集，要与数据库的一致，否则中文会出现乱码
void connection::character(char *charset)
{
  PQsetClientEncoding(m_conn, charset);

  return;
}

int connection::disconnect()
{
  memset(&m_cda,0,sizeof(m_cda));

  if (m_state == 0)
  {
    m_cda.rc=-1; strcpy(m_cda.message,"database not open."); return -1;
  }

  rollback();

  PQfinish(m_conn);

  m_conn=0;

  m_state = 0;

  return 0;
}

// 开始事务
int connection::begin()
{
  memset(&m_cda,0,sizeof(m_cda));

  if (m_state == 0)
  {
    m_cda.rc=-1; strcpy(m_cda.message,"database not open."); return -1;
  }

  if (m_autocommitopt==1) return 0;

  PGresult *m_res = PQexec(m_conn,"BEGIN");

  if (PQresultStatus(m_res) != PGRES_COMMAND_OK)
  {
    m_cda.rc=-1; strcpy(m_cda.message,PQerrorMessage(m_conn)); 
  }

  PQclear(m_res); m_res=0;

  return m_cda.rc;
}

int connection::commit()
{
  memset(&m_cda,0,sizeof(m_cda));

  if (m_state == 0)
  {
    m_cda.rc=-1; strcpy(m_cda.message,"database not open."); return -1;
  }

  if (m_autocommitopt==1) return 0;

  PGresult *m_res = PQexec(m_conn,"COMMIT");

  if (PQresultStatus(m_res) != PGRES_COMMAND_OK)
  {
    m_cda.rc=-1; strcpy(m_cda.message,PQerrorMessage(m_conn));
  }

  PQclear(m_res); m_res=0;

  begin();

  return m_cda.rc;
}

int connection::rollback()
{
  memset(&m_cda,0,sizeof(m_cda));

  if (m_state == 0)
  {
    m_cda.rc=-1; strcpy(m_cda.message,"database not open."); return -1;
  }

  if (m_autocommitopt==1) return 0;

  PGresult *m_res = PQexec(m_conn,"ROLLBACK");

  if (PQresultStatus(m_res) != PGRES_COMMAND_OK)
  {
    m_cda.rc=-1; strcpy(m_cda.message,PQerrorMessage(m_conn));
  }

  PQclear(m_res); m_res=0;

  begin();

  return m_cda.rc;
}

int connection::execute(const char *fmt,...)
{
  memset(m_sql,0,sizeof(m_sql));

  va_list ap;
  va_start(ap,fmt);
  vsnprintf(m_sql,10240,fmt,ap);
  va_end(ap);

  sqlstatement stmt(this);

  return stmt.execute(m_sql);
}

sqlstatement::sqlstatement()
{
  initial();
}

void sqlstatement::initial()
{
  m_state=0;

  memset(&m_cda,0,sizeof(m_cda));

  memset(m_sql,0,sizeof(m_sql));

  m_res=0;

  m_conn=0;

  m_cda.rc=-1;

  strcpy(m_cda.message,"sqlstatement not connect to connection.\n");

  m_nParamsIn=0;
  memset(m_paramValuesIn,0,sizeof(m_paramValuesIn));
  memset(m_paramValuesInVal,0,sizeof(m_paramValuesInVal));
  memset(m_paramValuesInPtr,0,sizeof(m_paramValuesInPtr));
  memset(m_paramLengthsIn,0,sizeof(m_paramLengthsIn));

  for (int ii=0;ii<MAXPARAMS;ii++)
  {
    m_paramValuesIn[ii]=m_paramValuesInVal[ii];
  }

  m_respos=0;

  m_restotal=0;

  m_nParamsOut=0;
}

sqlstatement::sqlstatement(connection *in_conn)
{
  initial();
  connect(in_conn);
}

sqlstatement::~sqlstatement()
{
  disconnect();
}

int sqlstatement::disconnect()
{
  if (m_state == 0) return 0;

  memset(&m_cda,0,sizeof(m_cda));

  if (m_res!= 0) { PQclear(m_res); m_res=0; }

  m_state=0;

  memset(&m_cda,0,sizeof(m_cda));

  memset(m_sql,0,sizeof(m_sql));

  m_cda.rc=-1;
  strcpy(m_cda.message,"cursor not open.");

  PQclear(m_res); m_res=0;

  m_nParamsIn=0;
  memset(m_paramValuesInVal,0,sizeof(m_paramValuesInVal));
  memset(m_paramLengthsIn,0,sizeof(m_paramLengthsIn));
  memset(m_paramTypesIn,0,sizeof(m_paramTypesIn));

  m_respos=0;
  m_restotal=0;
  m_nParamsOut=0;
  memset(m_paramValuesOut,0,sizeof(m_paramValuesOut));
  memset(m_paramTypesOut,0,sizeof(m_paramTypesOut));
  memset(m_paramLengthsOut,0,sizeof(m_paramLengthsOut));

  return 0;
}

int sqlstatement::connect(connection *in_conn)
{
  // 注意，一个sqlstatement在程序中只能连一个connection，不允许连多个connection
  // 所以，只要这个光标已打开，就不允许再次打开，直接返回成功
  if ( m_state == 1 ) return 0;

  memset(&m_cda,0,sizeof(m_cda));

  m_conn=in_conn;

  // 如果数据库连接类的指针为空，直接返回失败
  if (m_conn == 0)
  {
    m_cda.rc=-1; strcpy(m_cda.message,"database not open.\n"); return -1;
  }

  // 如果数据库没有连接好，直接返回失败
  if (m_conn->m_state == 0)
  {
    m_cda.rc=-1; strcpy(m_cda.message,"database not open.\n"); return -1;
  }

  m_state = 1;  // 光标成功打开

  return 0;
}

// 把字符串中的小写字母转换成大写，忽略不是字母的字符。
// 这个函数只在prepare方法中用到。
void PG__ToUpper(char *str)
{
  if (str == 0) return;

  if (strlen(str) == 0) return;

  int istrlen=strlen(str);

  for (int ii=0;ii<istrlen;ii++)
  {
    if ( (str[ii] >= 'a') && (str[ii] <= 'z') ) str[ii]=str[ii] - 32;
  }
}

// 删除字符串左边的空格。
// 这个函数只在prepare方法中用到。
void PG__DeleteLChar(char *str,const char chr)
{
  if (str == 0) return;
  if (strlen(str) == 0) return;

  char strTemp[strlen(str)+1];

  int iTemp=0;

  memset(strTemp,0,sizeof(strTemp));
  strcpy(strTemp,str);

  while ( strTemp[iTemp] == chr )  iTemp++;

  memset(str,0,strlen(str)+1);

  strcpy(str,strTemp+iTemp);

  return;
}

// 字符串替换函数
// 在字符串str中，如果存在字符串str1，就替换为字符串str2。
// 这个函数只在prepare方法中用到。
void PG__UpdateStr(char *str,const char *str1,const char *str2,bool bloop)
{
  if (str == 0) return;
  if (strlen(str) == 0) return;
  if ( (str1 == 0) || (str2 == 0) ) return;

  // 如果bloop为true并且str2中包函了str1的内容，直接返回，因为会进入死循环，最终导致内存溢出。
  if ( (bloop==true) && (strstr(str2,str1)>0) ) return;

  // 尽可能分配更多的空间，但仍有可能出现内存溢出的情况，最好优化成string。
  int ilen=strlen(str)*10;
  if (ilen<1000) ilen=1000;

  char strTemp[ilen];

  char *strStart=str;

  char *strPos=0;

  while (true)
  {
    if (bloop == true)
    {
      strPos=strstr(str,str1);
    }
    else
    {
      strPos=strstr(strStart,str1);
    }

    if (strPos == 0) break;

    memset(strTemp,0,sizeof(strTemp));
    strncpy(strTemp,str,strPos-str);
    strcat(strTemp,str2);
    strcat(strTemp,strPos+strlen(str1));
    strcpy(str,strTemp);

    strStart=strPos+strlen(str2);
  }
}

int sqlstatement::prepare(const char *fmt,...)
{
  memset(&m_cda,0,sizeof(m_cda));

  if (m_state == 0)
  {
    m_cda.rc=-1; strcpy(m_cda.message,"cursor not open.\n"); return -1;
  }

  memset(m_sql,0,sizeof(m_sql));

  va_list ap;
  va_start(ap,fmt);
  vsnprintf(m_sql,10240,fmt,ap);
  va_end(ap);

  int ilen=strlen(m_sql);

  // PG数据库与Oracle不同，是采用$，不是:
  for (int ii=0;ii<ilen;ii++)
  {
    if ( (m_sql[ii]==':') && (isdigit(m_sql[ii+1])!=0) ) m_sql[ii]='$';
  }

  // 判断是否是查询语句，如果是，把m_sqltype设为0，其它语句设为1。
  m_sqltype=1;

  // 从待执行的SQL语句中截取30个字符，如果是以"select"打头，就认为是查询语句。
  char strtemp[31]; memset(strtemp,0,sizeof(strtemp)); strncpy(strtemp,m_sql,30);
  PG__ToUpper(strtemp); PG__DeleteLChar(strtemp,' ');
  if (strncmp(strtemp,"SELECT",6)==0)  m_sqltype=0;

  // 为了和oracle兼容，把to_date替换成to_timestamp。
  PG__UpdateStr(m_sql,"to_date","to_timestamp",false);

  PQclear(m_res); m_res=0;

  m_nParamsIn=0;
  // m_paramValuesIn在这里不能被初始化
  memset(m_paramValuesInVal,0,sizeof(m_paramValuesInVal));
  memset(m_paramValuesInPtr,0,sizeof(m_paramValuesInPtr));
  memset(m_paramLengthsIn,0,sizeof(m_paramLengthsIn));
  memset(m_paramTypesIn,0,sizeof(m_paramTypesIn));

  m_respos=0;
  m_restotal=0;
  m_nParamsOut=0;
  memset(m_paramValuesOut,0,sizeof(m_paramValuesOut));
  memset(m_paramTypesOut,0,sizeof(m_paramTypesOut));
  memset(m_paramLengthsOut,0,sizeof(m_paramLengthsOut));

  return 0;
}

int sqlstatement::bindin(unsigned int position,int *value)
{
  if (m_nParamsIn>=MAXPARAMS) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"int");

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(unsigned int position,long *value)
{
  if (m_nParamsIn>=MAXPARAMS) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"long");

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(unsigned int position,unsigned int *value)
{
  if (m_nParamsIn>=MAXPARAMS) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"unsigned int");

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(unsigned int position,unsigned long *value)
{
  if (m_nParamsIn>=MAXPARAMS) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"unsigned long");

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(unsigned int position,float *value)
{
  if (m_nParamsIn>=MAXPARAMS) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"float");

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(unsigned int position,double *value)
{
  if (m_nParamsIn>=MAXPARAMS) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"double");

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(unsigned int position,char *value,unsigned int len)
{
  if (m_nParamsIn>=MAXPARAMS) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=len;
  strcpy(m_paramTypesIn[m_nParamsIn],"char");

  if (len>MAXFIELDLENGTH) m_paramLengthsIn[m_nParamsIn]=MAXFIELDLENGTH;

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindout(unsigned int position,int *value)
{
  if (m_nParamsOut>=MAXPARAMS) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"int");
  m_paramLengthsOut[m_nParamsOut]=sizeof(int);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(unsigned int position,long *value)
{
  if (m_nParamsOut>=MAXPARAMS) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"long");
  m_paramLengthsOut[m_nParamsOut]=sizeof(long);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(unsigned int position,unsigned int *value)
{
  if (m_nParamsOut>=MAXPARAMS) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"unsigned int");
  m_paramLengthsOut[m_nParamsOut]=sizeof(unsigned int);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(unsigned int position,unsigned long *value)
{
  if (m_nParamsOut>=MAXPARAMS) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"unsigned long");
  m_paramLengthsOut[m_nParamsOut]=sizeof(unsigned long);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(unsigned int position,float *value)
{
  if (m_nParamsOut>=MAXPARAMS) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"float");
  m_paramLengthsOut[m_nParamsOut]=sizeof(float);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(unsigned int position,double *value)
{
  if (m_nParamsOut>=MAXPARAMS) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"double");
  m_paramLengthsOut[m_nParamsOut]=sizeof(double);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(unsigned int position,char *value,unsigned int len)
{
  if (m_nParamsOut>=MAXPARAMS) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"char");
  m_paramLengthsOut[m_nParamsOut]=len;

  m_nParamsOut++;

  return 0;
}



int sqlstatement::execute()
{
  memset(&m_cda,0,sizeof(m_cda));

  if (m_state == 0)
  {
    m_cda.rc=-1; strcpy(m_cda.message,"cursor not open.\n"); return -1;
  }

  /*
  // 判断是否是查询语句，如果是，把m_sqltype设为0，其它语句设为1。
  m_sqltype=1;

  // 从待执行的SQL语句中截取15个字符，如果这15个字符中包括了“select”，就认为是查询语句
  char strtemp[16]; memset(strtemp,0,sizeof(strtemp)); strncpy(strtemp,m_sql,15);

  for (unsigned int ii=0;ii<strlen(strtemp);ii++)
  {
    if ( (strtemp[ii] >= 97) && (strtemp[ii] <= 122) ) strtemp[ii]=strtemp[ii] - 32;
  }

  if ( (strstr(strtemp,"select") > 0) || (strstr(strtemp,"SELECT") > 0) ) m_sqltype=0;
  */

  PQclear(m_res); m_res=0;

  m_respos=0;
  m_restotal=0;

  // 如果有输入参数
  if (m_nParamsIn > 0)
  {
    memset(m_paramValuesInVal,0,sizeof(m_paramValuesInVal));

    for (int ii=0;ii<m_nParamsIn;ii++)
    {
      if (strcmp(m_paramTypesIn[ii],"int") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%d",(int)(*(int *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"long") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%ld",(long)(*(long *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"unsigned int") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%u",(unsigned int)(*(unsigned int *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"unsigned long") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%lu",(unsigned long)(*(unsigned long *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"float") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%f",(float)(*(float *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"double") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%f",(double)(*(double *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"char") == 0)
      {
        strncpy(m_paramValuesInVal[ii],m_paramValuesInPtr[ii],m_paramLengthsIn[ii]); 
        m_paramValuesInVal[ii][m_paramLengthsIn[ii]]=0;
      }
    }
  }

  m_res = PQexecParams(\
             (PGconn *)m_conn->m_conn,
         (const char *)m_sql,
                  (int)m_nParamsIn,       // one param 
          (const Oid *)0,    // let the backend deduce param type 
 (const char * const *)m_paramValuesIn,
          (const int *)m_paramLengthsIn,    // don't need param lengths since text 
          (const int *)0,    // default to all text params 
                  (int)0);      // ask for binary results 

  // 如果不是查询语句
  if (m_sqltype == 1)
  {
    if (PQresultStatus(m_res) != PGRES_COMMAND_OK)
    {
      m_cda.rc=-1; strcpy(m_cda.message,PQerrorMessage(m_conn->m_conn)); 

      memcpy(&m_conn->m_cda,&m_cda,sizeof(m_cda)); 

      PQclear(m_res); m_res=0;

      return -1;
    }

    // 影响记录的行数
    m_cda.rpc = atoi(PQcmdTuples(m_res));
    m_conn->m_cda.rpc=m_cda.rpc;
  }
  else
  {
    // 如果是查询语句
    if (PQresultStatus(m_res) != PGRES_TUPLES_OK)
    {
      m_cda.rc=-1; strcpy(m_cda.message,PQerrorMessage(m_conn->m_conn)); 

      memcpy(&m_conn->m_cda,&m_cda,sizeof(m_cda)); 

      return -1;
    }

    m_respos=0; m_restotal=PQntuples(m_res); 
  }

  return 0;
}

int sqlstatement::execute(const char *fmt,...)
{
  char strtmpsql[10241];
  memset(strtmpsql,0,sizeof(strtmpsql));

  va_list ap;
  va_start(ap,fmt);
  vsnprintf(strtmpsql,10240,fmt,ap);
  va_end(ap);

  if (prepare(strtmpsql) != 0) return m_cda.rc;

  return execute();
}

int sqlstatement::next()
{
  // 注意，在该函数中，不可用memset(&m_cda,0,sizeof(m_cda))，否则会清空m_cda.rpc的内容
  if (m_state == 0)
  {
    m_cda.rc=-1; strcpy(m_cda.message,"cursor not open.\n"); return -1;
  }

  // 如果语句未执行成功，直接返回失败。
  if (m_cda.rc != 0) return m_cda.rc;

  // 判断是否是查询语句，如果不是，直接返回错误
  if (m_sqltype != 0)
  {
    m_cda.rc=-1; strcpy(m_cda.message,"no recordset found.\n"); return -1;
  }

  if (m_cda.rpc>=(unsigned long)PQntuples(m_res))
  {
    m_cda.rc=1403; strcpy(m_cda.message,"no data found"); return m_cda.rc;
  }

  for (int ii=0;ii<m_nParamsOut;ii++)
  {
    if (strcmp(m_paramTypesOut[ii],"int" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,m_cda.rpc,ii);
      int ival = atoi(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"long" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,m_cda.rpc,ii);
      long ival = atol(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"unsigned int" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,m_cda.rpc,ii);
      unsigned int ival = (unsigned int)atoi(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"unsigned long" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,m_cda.rpc,ii);
      unsigned long ival = (unsigned long)atol(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"float" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,m_cda.rpc,ii);
      float ival = atof(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"double" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,m_cda.rpc,ii);
      double ival = atof(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"char") == 0)  
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);
      
      int len=0;
      if (m_paramLengthsOut[ii]>PQgetlength(m_res,m_cda.rpc,ii))
        len=PQgetlength(m_res,m_cda.rpc,ii);
      else
        len=m_paramLengthsOut[ii];
      
      strncpy(m_paramValuesOut[ii],PQgetvalue(m_res,m_cda.rpc,ii),len);

      m_paramValuesOut[ii][len]=0;
    }
  }

  m_cda.rpc++;

  m_conn->m_cda.rpc=m_cda.rpc;

  return m_cda.rc;
}


