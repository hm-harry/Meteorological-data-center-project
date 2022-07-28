/*
    程序名：idcapp.cpp 此程序是数据中心项目公用函数和类的实现文件
    作者： 吴惠明
*/

#include "idcapp.h"

CZHOBTMIND::CZHOBTMIND(){
    m_conn = 0;
    m_logfile = 0;
}

CZHOBTMIND::CZHOBTMIND(connection* conn, CLogFile* logfile){
    m_conn = conn;
    m_logfile = logfile;
}

CZHOBTMIND::~CZHOBTMIND(){}

void CZHOBTMIND::BindConnLog(connection* conn, CLogFile* logfile){
    m_conn = conn;
    m_logfile = logfile;
}

// 把文件读到的一行数据拆分到m_zhobtmind结构体中
bool CZHOBTMIND::SplitBuffer(char* strBuffer, bool bisxml){
    memset(&m_zhobtmind, 0, sizeof(struct st_zhobtmind));

    if(bisxml == true){
        GetXMLBuffer(strBuffer, "obtid", m_zhobtmind.obtid, 10);
        GetXMLBuffer(strBuffer, "ddatetime", m_zhobtmind.ddatetime, 14);
        char tmp[11];
        GetXMLBuffer(strBuffer, "t", tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.t, 10, "%d", (int)(atof(tmp)*10));
        GetXMLBuffer(strBuffer, "p", tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.p, 10, "%d", (int)(atof(tmp)*10));
        GetXMLBuffer(strBuffer, "u", m_zhobtmind.u, 10);
        GetXMLBuffer(strBuffer, "wd", m_zhobtmind.wd, 10);
        GetXMLBuffer(strBuffer, "wf", tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.wf, 10, "%d", (int)(atof(tmp)*10));
        GetXMLBuffer(strBuffer, "r", tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.r, 10, "%d", (int)(atof(tmp)*10));
        GetXMLBuffer(strBuffer, "vis", tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.vis, 10, "%d", (int)(atof(tmp)*10));
    }else{
        CCmdStr CmdStr;
        CmdStr.SplitToCmd(strBuffer, ",");
        CmdStr.GetValue(0, m_zhobtmind.obtid, 10);
        CmdStr.GetValue(1, m_zhobtmind.ddatetime, 14);
        char tmp[11];
        CmdStr.GetValue(2, tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.t, 10, "%d", (int)(atof(tmp)*10));
        CmdStr.GetValue(3, tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.p, 10, "%d", (int)(atof(tmp)*10));
        CmdStr.GetValue(4, m_zhobtmind.u, 10);
        CmdStr.GetValue(5, m_zhobtmind.wd, 10);
        CmdStr.GetValue(6, tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.wf, 10, "%d", (int)(atof(tmp)*10));
        CmdStr.GetValue(7, tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.r, 10, "%d", (int)(atof(tmp)*10));
        CmdStr.GetValue(8, tmp, 10); if(strlen(tmp) > 0) snprintf(m_zhobtmind.vis, 10, "%d", (int)(atof(tmp)*10));
    }
    

    STRCPY(m_buffer, sizeof(m_buffer), strBuffer);

    return true;
}

// 把m_zhobtmind结构体中的数据插入T_ZHOBTMIND表中
bool CZHOBTMIND::InsertTable(){
    if(m_stmt.m_state == 0){
        m_stmt.connect(m_conn);
        m_stmt.prepare("insert into T_ZHOBTMIND(obtid, ddatetime, t, p, u, wd, wf, r, vis) values(:1, str_to_date(:2, '%%Y%%m%%d%%H%%i%%s'), :3, :4, :5, :6, :7, :8, :9)");
        m_stmt.bindin(1, m_zhobtmind.obtid, 10);
        m_stmt.bindin(2, m_zhobtmind.ddatetime, 14);
        m_stmt.bindin(3, m_zhobtmind.t, 10);
        m_stmt.bindin(4, m_zhobtmind.p, 10);
        m_stmt.bindin(5, m_zhobtmind.u, 10);
        m_stmt.bindin(6, m_zhobtmind.wd, 10);
        m_stmt.bindin(7, m_zhobtmind.wf, 10);
        m_stmt.bindin(8, m_zhobtmind.r, 10);
        m_stmt.bindin(9, m_zhobtmind.vis, 10);
    }
    // 把结构体中的数据插入表中
    if(m_stmt.execute() != 0){
        if(m_stmt.m_cda.rc != 1062){
            m_logfile->Write("Buffer = %s\n", m_buffer);
            m_logfile->Write("m_stmt.execute() failed. \n%s\n%s\n", m_stmt.m_sql, m_stmt.m_cda.message);
        }
        return false;
    }
    return true;
}