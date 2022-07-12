/*
 *  程序名：demo8.cpp，此程序演示开发框架中SNPRINTF函数的使用。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  char str[26];   // 字符串str的大小是11字节。

  SNPRINTF(str,sizeof(str),5,"messi");
  printf("str=%s=\n",str);    // 出输结果是str=mess=

  SNPRINTF(str,sizeof(str),9,"name:%s,no:%d,job:%s","messi",10,"striker");
  printf("str=%s=\n",str);    // 出输结果是str=name:mes=

  SNPRINTF(str,sizeof(str),30,"name:%s,no:%d,job:%s","messi",10,"striker");
  printf("str=%s=\n",str);    // 出输结果是str=name:messi,no:10,job:stri=
}

