DELETE FROM T_ZHOBTMIND where ddatetime < timestampadd(minute, -120, now());
DELETE FROM T_ZHOBTMIND1 where ddatetime < timestampadd(minute, -120, now());
DELETE FROM T_ZHOBTMIND2 where ddatetime < timestampadd(minute, -120, now());
DELETE FROM T_ZHOBTMIND3 where ddatetime < timestampadd(minute, -120, now());
