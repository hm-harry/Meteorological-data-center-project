/*
 *  程序名：demo37.cpp，此程序演示开发框架中FGETS函数的用法。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  FILE *fp=0;

  if ( (fp=FOPEN("/tmp/aaa/bbb/ccc/tmp.xml","r"))==0)
  {
    printf("FOPEN(/tmp/aaa/bbb/ccc/tmp.xml) %d:%s\n",errno,strerror(errno)); return -1;
  }

  char strBuffer[301];

  while (true)
  {
    memset(strBuffer,0,sizeof(strBuffer));
    //if (fgets(strBuffer,300,fp)==false) break;     // 行内容以"\n"结束。
    if (FGETS(fp,strBuffer,300,"<endl/>")==false) break; // 行内容以"<endl/>"结束。

    printf("strBuffer=%s",strBuffer);
  }

  fclose(fp);
}

