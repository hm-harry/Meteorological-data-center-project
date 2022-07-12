insert into T_DATATYPE(typeid,ptypeid,typename,keyid) values('01',null,'国家基本站',SEQ_DATATYPE.nextval);
insert into T_DATATYPE(typeid,ptypeid,typename,keyid) values('02',null,'预报产品'  ,SEQ_DATATYPE.nextval);
insert into T_DATATYPE(typeid,ptypeid,typename,keyid) values('03',null,'预警信号'  ,SEQ_DATATYPE.nextval);
insert into T_DATATYPE(typeid,ptypeid,typename,keyid) values('04',null,'雷达产品'  ,SEQ_DATATYPE.nextval);

insert into T_DATATYPE(typeid,ptypeid,typename,keyid) values('0101','01','全国站点参数',SEQ_DATATYPE.nextval);
insert into T_DATATYPE(typeid,ptypeid,typename,keyid) values('0102','01','分钟观测数据',SEQ_DATATYPE.nextval);
insert into T_DATATYPE(typeid,ptypeid,typename,keyid) values('0103','01','日统计数据',SEQ_DATATYPE.nextval);

insert into T_DATATYPE(typeid,ptypeid,typename,keyid) values('0301','03','本市预警信号'  ,SEQ_DATATYPE.nextval);
insert into T_DATATYPE(typeid,ptypeid,typename,keyid) values('0302','03','分区预警信号'  ,SEQ_DATATYPE.nextval);
insert into T_DATATYPE(typeid,ptypeid,typename,keyid) values('0303','03','全国预警信号'  ,SEQ_DATATYPE.nextval);

exit;
