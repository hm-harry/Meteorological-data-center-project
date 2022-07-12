delete from T_USERANDINTER;
insert into T_USERANDINTER select T_USERINFO.username,T_INTERCFG.intername from T_USERINFO,T_INTERCFG where T_USERINFO.rsts=1 and T_INTERCFG.rsts=1;

exit;
