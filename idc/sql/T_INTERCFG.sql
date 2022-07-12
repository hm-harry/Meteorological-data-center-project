delete from T_INTERCFG;

/* 获取全部的站点参数。*/
insert into T_INTERCFG(typeid,intername,intercname,selectsql,colstr,bindin,keyid) 
values ('0101','getzhobtcode','全国站点参数','select obtid,cityname,provname,lat,lon,height from T_ZHOBTCODE','obtid,cityname,provname,lat,lon,height',null,SEQ_INTERCFG.nextval);

/* 按站点获取全国站点分钟观测数据。 */
insert into T_INTERCFG(typeid,intername,intercname,selectsql,colstr,bindin,keyid) 
values ('0102','getzhobtmind1','全国站点分钟观测数据(按站点)','select obtid,to_char(ddatetime,''yyyymmddhh24miss''),t,p,u,wd,wf,r,vis from T_ZHOBTMIND where obtid=:1','obtid,ddatetime,t,p,u,wd,wf,r,vis','obtid',SEQ_INTERCFG.nextval);

/* 按时间段获取全国站点分钟观测数据。 */
insert into T_INTERCFG(typeid,intername,intercname,selectsql,colstr,bindin,keyid) 
values ('0102','getzhobtmind2','全国站点分钟观测数据(按时间段)','select obtid,to_char(ddatetime,''yyyymmddhh24miss''),t,p,u,wd,wf,r,vis from T_ZHOBTMIND where ddatetime>=to_date(:1,''yyyymmddhh24miss'') and ddatetime<=to_date(:2,''yyyymmddhh24miss'')','obtid,ddatetime,t,p,u,wd,wf,r,vis','begintime,endtime',SEQ_INTERCFG.nextval);

/* 按站点和时间段获取全国站点分钟观测数据。 */
insert into T_INTERCFG(typeid,intername,intercname,selectsql,colstr,bindin,keyid) 
values ('0102','getzhobtmind3','全国站点分钟观测数据(按站点和时间段)','select obtid,to_char(ddatetime,''yyyymmddhh24miss''),t,p,u,wd,wf,r,vis from T_ZHOBTMIND where obtid=:1 and ddatetime>=to_date(:2,''yyyymmddhh24miss'') and ddatetime<=to_date(:3,''yyyymmddhh24miss'')','obtid,ddatetime,t,p,u,wd,wf,r,vis','obtid,begintime,endtime',SEQ_INTERCFG.nextval);

exit;

