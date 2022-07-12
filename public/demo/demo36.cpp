/*
 *  程序名：demo36.cpp，此程序演示开发框架中FOPEN函数的用法。
 *  作者：吴从周
*/
#include "../_public.h"

int main()
{
  FILE *fp=0;

  // 用FOPEN函数代替fopen库函数，如果目录/tmp/aaa/bbb/ccc不存在，会创建它。
  if ( (fp=FOPEN("/tmp/aaa/bbb/ccc/tmp.xml","w"))==0)   
  {
    printf("FOPEN(/tmp/aaa/bbb/ccc/tmp.xml) %d:%s\n",errno,strerror(errno)); return -1;
  }

  // 向文件中写入两行超女数据。
  fprintf(fp,"<data>\n"\
     "<name>妲已</name><age>28</age><sc>火辣</sc><yz>漂亮</yz><memo>商要亡，关我什么事。</memo><endl/>\n"\
     "<name>西施</name><age>25</age><sc>火辣</sc><yz>漂亮</yz><memo>1、中国排名第一的美女；\n"\
     "2、男朋友是范蠡；\n"\
     "3、老公是夫差，被勾践弄死了。</memo><endl/>\n"\
     "</data>\n");

  fclose(fp);  // 关闭文件。
}

