/*
    程序名：crtsurfdata2.cpp 本程序用于生成全国气象站点观测的分钟数据
    作者：吴惠明
*/

#include "_public.h"

// 日志类
CLogFile logfile;

// 全国气象站点参数结构体
struct st_stcode{
  char provname[31]; // 省
  char obtid[11]; // 站号
  char obtname[31]; // 站名
  double lat; // 纬度
  double lon; // 经度
  double height; // 海拔高度
};

// 存放全国气象站点参数的容器
vector<struct st_stcode> vstcode;

// 把站点参数文件加载到vstcode容器中
bool LoadSTCode(const char* inifile);

int main(int argc, char* argv[]){
  // inifile outpath logfile
  if(argc != 4){
    printf("Using:./crtsurfdatal inifile outpath logfile\n");
    printf("Example:/project/idc1/bin/crtsurfdata2 /project/idc1/ini/stcode.ini /tmp/sufdata /log/idc/crtsurfdata2.log\n\n");
    printf("inifile 全国气象站点参数文件名。\n");
    printf("outpath 全国气象站点数据文件存放的目录。\n");
    printf("logfile 本程序运行的日志文件名。\n\n");

    return -1;
  }

  if(logfile.Open(argv[3], "a+", false) == false){
    printf("logfile.Open(%s) filed.\n", argv[3]);
    return -1;
  }
  logfile.Write("crtsurfdata2 开始运行。\n");

  //这里插入业务代码
  

  logfile.Write("crtsurfdata2 运行结束。\n"); 

  if(LoadSTCode(argv[1]) == false) return -1;
  return 0;
}

// 把站点参数文件加载到vstcode容器中
bool LoadSTCode(const char* inifile){
  CFile File;
  // 打开站点文件
  if(File.Open(inifile, "r") == false){
    logfile.Write("File.Open(%s) failed.\n", inifile);
    return false;
  }
  char strBuffer[301];
  
  CCmdStr CmdStr;
  
  struct st_stcode stcode;
  while(true){
    // 从站点参数文件中读取一行，如果读完，跳出循环
    // Fgets中有写 memset(strBuffer, 0, sizeof(strBuffer));
    if(File.Fgets(strBuffer, 300, true) == false) break;
    
    // 把读到的一行拆分
    CmdStr.SplitToCmd(strBuffer, ",", true);
    
    // 删除无效行
    if(CmdStr.CmdCount() != 6) continue;

    // 把站点参数的每个数据项保存到站点参数结构体中
    memset(&stcode,0,sizeof(struct st_stcode));
    CmdStr.GetValue(0, stcode.provname, 30);
    CmdStr.GetValue(1, stcode.obtid, 10);
    CmdStr.GetValue(2, stcode.obtname, 30);
    CmdStr.GetValue(3, &stcode.lat);
    CmdStr.GetValue(4, &stcode.lon);
    CmdStr.GetValue(5, &stcode.height);

    // 把站点参数结构体放入站点参数容器
    vstcode.push_back(stcode);
  }
  /*
  for(int ii = 0; ii < vstcode.size(); ++ii){
    logfile.Write("provname = %s, obtid = %s, obtname = %s, lat = %.2f, lon = %.2f, height = %.2f\n", vstcode[ii].provname, vstcode[ii].obtid, vstcode[ii].obtname, vstcode[ii].lat, vstcode[ii].lon, vstcode[ii].height);
  }
  */
  return true;
}
