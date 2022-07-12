/*
 *  程序名：demo12.cpp，此程序演示开发框架中字符串大小写转换函数的使用。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  char str1[31];   // C语言风格的字符串。

  STRCPY(str1,sizeof(str1),"12abz45ABz8西施。");
  ToUpper(str1);      // 把str1中的小写字母转换为大写。
  printf("str1=%s=\n",str1);    // 出输结果是str1=12ABZ45ABZ8西施。=

  STRCPY(str1,sizeof(str1),"12abz45ABz8西施。");
  ToLower(str1);      // 把str1中的大写字母转换为小写。
  printf("str1=%s=\n",str1);    // 出输结果是str1=12abz45abz8西施。=

  string str2;    // C++语言风格的字符串。
  
  str2="12abz45ABz8西施。";
  ToUpper(str2);      // 把str2中的小写字母转换为大写。
  printf("str2=%s=\n",str2.c_str());    // 出输结果是str2=12ABZ45ABZ8西施。=

  str2="12abz45ABz8西施。";
  ToLower(str2);      // 把str2中的大写字母转换为小写。
  printf("str2=%s=\n",str2.c_str());    // 出输结果是str1=12abz45abz8西施。=
}

