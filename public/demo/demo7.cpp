/*
 *  程序名：demo7.cpp，此程序演示开发框架中SPRINTF函数的使用。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  char str[21];   // 字符串str的大小是21字节。

  SPRINTF(str,sizeof(str),"name:%s,no:%d","messi",10);
  printf("str=%s=\n",str);    // 出输结果是str=name:messi,no:10=

  SPRINTF(str,sizeof(str),"name:%s,no:%d,job:%s","messi",10,"striker");
  printf("str=%s=\n",str);    // 出输结果是str=name:messi,no:10,job=
}

