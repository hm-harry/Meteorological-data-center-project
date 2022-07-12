/*
 *  程序名：demo21.cpp，此程序演示开发框架中字符串替换UpdateStr函数的使用。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  char str[301];

  STRCPY(str,sizeof(str),"name:messi,no:10,job:striker.");
  UpdateStr(str,":","=");     // 把冒号替换成等号。
  printf("str=%s=\n",str);    // 出输结果是str1=name=messi,no=10,job=striker.=

  STRCPY(str,sizeof(str),"name:messi,no:10,job:striker.");
  UpdateStr(str,"name:","");    // 把"name:"替换成""，相当于删除内容"name:"。
  printf("str=%s=\n",str);      // 出输结果是str1=messi,no:10,job:striker.=

  STRCPY(str,sizeof(str),"messi----10----striker");  
  UpdateStr(str,"--","-",false);    // 把两个"--"替换成一个"-"，bLoop参数为false。
  printf("str=%s=\n",str);          // 出输结果是str1=messi--10--striker=

  STRCPY(str,sizeof(str),"messi----10----striker");  
  UpdateStr(str,"--","-",true);    // 把两个"--"替换成一个"-"，bLoop参数为true。
  printf("str=%s=\n",str);         // 出输结果是str1=messi-10-striker=

  STRCPY(str,sizeof(str),"messi-10-striker");  
  UpdateStr(str,"-","--",false);    // 把一个"-"替换成两个"--"，bLoop参数为false。
  printf("str=%s=\n",str);          // 出输结果是str1=messi--10--striker=

  STRCPY(str,sizeof(str),"messi-10-striker");  

  // 以下代码把"-"替换成"--"，bloop参数为true，存在逻辑错误，UpdateStr将不执行替换。
  UpdateStr(str,"-","--",true);    // 把一个"-"替换成两个"--"，bloop参数为true。
  printf("str=%s=\n",str);         // 出输结果是str1=messi-10-striker=
}

