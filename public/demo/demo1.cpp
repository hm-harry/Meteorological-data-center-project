/*
 * 程序名：demo1.cpp，此程序演示开发框架中STRCPY函数的使用。
 * 作者：吴从周
 */


#include "../_public.h"

int main(int argc,char *argv[])
{
  char str[11];   // 字符串str的大小是11字节。

  STRCPY(str,sizeof(str),"google");  // 待复制的内容没有超过str可以存放字符串的大小。
  printf("str=%s=\n",str);    // 出输结果是str=google=

  STRCPY(str,sizeof(str),"www.google.com");  // 待复制的内容超过了str可以存放字符串的大小。
  printf("str=%s=\n",str);    // 出输结果是str=www.google=
}

