/*
 *  程序名：demo5.cpp，此程序演示开发框架中STRNCAT函数的使用。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  char str[11];   // 字符串str的大小是11字节。

  STRCPY(str,sizeof(str),"www");  
  STRNCAT(str,sizeof(str),".free",10);  // str原有的内容加上待追加的内容没有超过str可以存放的大小。
  printf("str=%s=\n",str);              // 出输结果是str=www.free=

  STRCPY(str,sizeof(str),"www");  
  STRNCAT(str,sizeof(str),".freecplus.com",6);  // str原有的内容加上待追加的内容没有超过str可以存放的大小。
  printf("str=%s=\n",str);                      // 出输结果是str=www.freec=

  STRCPY(str,sizeof(str),"www");  
  STRNCAT(str,sizeof(str),".freecplus.com",10); // str原有的内容加上待追加的内容超过了str可以存放的大小。
  printf("str=%s=\n",str);                      // 出输结果是str=www.freecp=
}

