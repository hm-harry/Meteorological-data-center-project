/*
 *  程序名：demo20.cpp，此程序演示开发框架拆分字符串的类CCmdStr的使用。
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
  STRCPY(buffer,sizeof(buffer),"messi  ,10,true,30,68.5,2100000,Barcelona");

  CCmdStr CmdStr;
  CmdStr.SplitToCmd(buffer,",");        // 拆分buffer
  CmdStr.GetValue(0, stplayer.name,50); // 获取姓名
  CmdStr.GetValue(1, stplayer.no,5);    // 获取球衣号码
  CmdStr.GetValue(2,&stplayer.striker); // 场上位置
  CmdStr.GetValue(3,&stplayer.age);     // 获取年龄
  CmdStr.GetValue(4,&stplayer.weight);  // 获取体重
  CmdStr.GetValue(5,&stplayer.sal);     // 获取年薪，欧元。
  CmdStr.GetValue(6, stplayer.club,50); // 获取效力的俱乐部
  
  printf("name=%s,no=%s,striker=%d,age=%d,weight=%.1f,sal=%ld,club=%s\n",\
         stplayer.name,stplayer.no,stplayer.striker,stplayer.age,\
         stplayer.weight,stplayer.sal,stplayer.club);
  // 输出结果:name=messi,no=10,striker=1,age=30,weight=68.5,sal=21000000,club=Barcelona
}

