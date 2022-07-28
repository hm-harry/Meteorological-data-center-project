# 启动数据中心后台服务程序的脚本

# 检查服务是否超时，配置在/etc/rc.local中由root用户执行。
# /project/tools1/bin/procctl 30 /project/tools1/bin/checkproc

# 压缩数据中心后台服务程序的备份日志
/project/tools1/bin/procctl 300 /project/tools1/bin/gzipfiles /log/idc "*.log.20*" 0.04

# 生成用于测试的全国气象站点观测的分钟数据
/project/tools1/bin/procctl 60 /project/idc1/bin/crtsurfdata /project/idc1/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log xml,json,csv

# 清理原始的气象站点观测的分钟数据目录/tmp/idc/surfdata中的历史数据文件
/project/tools1/bin/procctl 300 /project/tools1/bin/deletefiles /tmp/idc/surfdata "*" 0.04

# 采集全国气象站点观测的分钟数据
/project/tools1/bin/procctl 30 /project/tools1/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log "<host>172.29.193.250</host><mode>1</mode><username>wuhm</username><password>whmhhh1998818</password><localpath>/idcdata/surfdata</localpath><remotepath>/tmp/idc/surfdata</remotepath><matchname>SURF_ZH*.XML</matchname><listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename><ptype>1</ptype><okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename><checkmtime>true</checkmtime><timeout>80</timeout><pname>ftpgetfiles_surfdata</pname>"

# 上传全国气象站点观测分钟数据的xml文件
/project/tools1/bin/procctl 30 /project/tools1/bin/ftpputfiles /log/idc/ftpputfiles_surfdata.log "<host>172.29.193.250</host><mode>1</mode><username>wuhm</username><password>whmhhh1998818</password><localpath>/tmp/idc/surfdata</localpath><remotepath>/tmp/ftpputtest</remotepath><matchname>SURF_ZH*.json</matchname><ptype>1</ptype><okfilename>/idcdata/ftplist/ftpputfiles_surfdata.xml</okfilename><timeout>80</timeout><pname>ftpputfiles_surfdata</pname>"

# 清理采集的气象站点观测的分钟数据目录/idcdata/surfdata中的历史数据文件
/project/tools1/bin/procctl 300 /project/tools1/bin/deletefiles /idcdata/surfdata "*" 0.04

# 清理采集的气象站点观测的分钟数据目录/tmp/ftpputtest中的历史数据文件
/project/tools1/bin/procctl 300 /project/tools1/bin/deletefiles /tmp/ftpputtest "*" 0.04

# 文件传输的服务端程序。
/project/tools1/bin/procctl 10 /project/tools1/bin/fileserver 5005 /log/idc/fileserver.log

# 把目录/tmp/ftpputtest中的文件上传到/tmp/tcpputtest目录中。
/project/tools1/bin/procctl 20 /project/tools1/bin/tcpputfiles /tmp/tcpputfiles_surfdata.log "<ip>127.0.0.1</ip><port>5005</port><ptype>1</ptype><clientpath>/tmp/ftpputtest</clientpath><clientpathbak>/tmp/tcp/surfdata1bak</clientpathbak><andchild>true</andchild><matchname>*.XML,*.CSV,*.JSON</matchname><srvpath>/tmp/tcpputtest</srvpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_surfdata</pname>"

# 把目录/tmp/tcpputtest中的文件下载到/tmp/tcpgettest目录中。
/project/tools1/bin/procctl 20 /project/tools1/bin/tcpgetfiles /log/idc/tcpgetfiles_surfdata.log "<ip>127.0.0.1</ip><port>5005</port><ptype>1</ptype><srvpath>/tmp/tcpputtest</srvpath><srvpathbak>/tmp/tcp/surfdata2bak</srvpathbak><andchild>true</andchild><matchname>*.XML,*.CSV,*.JSON</matchname><clientpath>/tmp/tcpgettest</clientpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpgetfiles_surfdata</pname>"

# 清理采集的全国气象站点观测的分钟数据目录/tmp/tcpgettest中的历史数据。
/project/tools1/bin/procctl 300 /project/tools1/bin/deletefiles /tmp/tcpgettest "*" 0.04

# 把全国站点参数数据保存到数据库表中，如果站点不存在则插入，站点已存在则更新。
/project/tools1/bin/procctl 120 /project/idc1/bin/obtcodetodb /project/idc1/ini/stcode.ini "127.0.0.1,root,whmhhh1998818,mysql,3306" utf8 /log/idc/obtcodetodb.log

# 本程序用于把全国站点分钟观测数据入库到T_ZHOBTMIND表中,数据只写入，不更新。
/project/tools1/bin/procctl 10 /project/idc1/bin/obtmindtodb /idcdata/surfdata "127.0.0.1,root,whmhhh1998818,mysql,3306" utf8 /log/idc/obtmindtodb.log

# 清理T_ZHOBTMIND表中120分钟之前的数据，防止磁盘被撑满
/project/tools1/bin/procctl 120 /project/tools1/bin/execsql /project/idc1/sql/cleardata.sql "127.0.0.1,root,whmhhh1998818,mysql,3306" utf8 /log/idc/execsql.log

# 每隔一个小时把T_ZHOBTCODE表的全部数据抽取出来
/project/tools1/bin/procctl 3600 /project/tools1/bin/dminingmysql /log/idc/dminingmysql_ZHOBTCODE.log     "<connstr>127.0.0.1,root,whmhhh1998818,mysql,3306</connstr><charset>utf8</charset><selectsql>Select obtid,cityname,provname,lat,lon,height from T_ZHOBTCODE</selectsql><fieldstr>obtid,cityname,provname,lat,lon,height</fieldstr><fieldlen>10,30,30,10,10,10</fieldlen><bfilename>ZHOBTCODE</bfilename><efilename>HYCZ</efilename><outpath>/idcdata/dmindata</outpath><timeout>30</timeout><pname>dminingmysql_ZHOBTCODE</pname>"

# 每30秒从T_ZHOBTMIND表中把增量抽取出来
/project/tools1/bin/procctl 30 /project/tools1/bin/dminingmysql /log/idc/dminingmysql_ZHOBTMIND.log     "<connstr>127.0.0.1,root,whmhhh1998818,mysql,3306</connstr><charset>utf8</charset><selectsql>Select obtid,date_format(ddatetime,'%%Y-%%m-%%d %%H:%%m:%%s'),t,p,u,wd,wf,r,vis,keyid from T_ZHOBTMIND where keyid>:1 and ddatetime>timestampadd(minute,-120,now())</selectsql><fieldstr>obtid,ddatetime,t,p,u,wd,wf,r,vis,keyid</fieldstr><fieldlen>10,19,8,8,8,8,8,8,8,15</fieldlen><bfilename>ZHOBTMIND</bfilename><efilename>HYCZ</efilename><outpath>/idcdata/dmindata</outpath><incfield>keyid</incfield><timeout>30</timeout><pname>dminingmysql_ZHOBTMIND_HYCZ</pname><maxcount>1000</maxcount><connstr1>127.0.0.1,root,whmhhh1998818,mysql,3306</connstr1>"

# 清理/idcdata/dmindata目录中文件，防止空间撑满
/project/tools1/bin/procctl 100 /project/tools1/bin/deletefiles /idcdata/dmindata "*" 0.02