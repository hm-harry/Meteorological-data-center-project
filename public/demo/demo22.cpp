/*
 *  程序名：demo22.cpp，此程序演示调用开发框架的GetXMLBuffer函数解析xml字符串。
 *  作者：吴从周
*/
#include "../_public.h"

// 用于存放足球运动员资料的结构体。
struct st_player
{
  char name[51];    // 姓名
  char no[6];       // 球衣号码
  bool striker;     // 场上位置是否是前锋，true-是；false-不是。
  int  age;         // 年龄
  double weight;    // 体重，kg。
  long sal;         // 年薪，欧元。
  char club[51];    // 效力的俱乐部
}stplayer;

int main()
{
  memset(&stplayer,0,sizeof(struct st_player));

  char buffer[301];  
  STRCPY(buffer,sizeof(buffer),"<name>西施</name><no>10</no><striker>true</striker><age>30</age><weight>68.5</weight><sal>21000000</sal><club>Barcelona</club>");

  GetXMLBuffer(buffer,"name",stplayer.name,50);
  GetXMLBuffer(buffer,"no",stplayer.no,5);
  GetXMLBuffer(buffer,"striker",&stplayer.striker);
  GetXMLBuffer(buffer,"age",&stplayer.age);
  GetXMLBuffer(buffer,"weight",&stplayer.weight);
  GetXMLBuffer(buffer,"sal",&stplayer.sal);
  GetXMLBuffer(buffer,"club",stplayer.club,50);

  printf("name=%s,no=%s,striker=%d,age=%d,weight=%.1f,sal=%ld,club=%s\n",\
         stplayer.name,stplayer.no,stplayer.striker,stplayer.age,\
         stplayer.weight,stplayer.sal,stplayer.club);
  // 输出结果:name=messi,no=10,striker=1,age=30,weight=68.5,sal=21000000,club=Barcelona
}

