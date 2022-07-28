# 开发框架头文件路径
PUBINCL = -I /project/public

# 开发框架cpp文件名，这里直接包含进来，没有才有链接库，是为了方便调试
PUBCPP = /project/public/_public.cpp

# mysql头文件存放路径
MYSQLINCL = -I /usr/local/mysql/include -I /project/public/db/mysql

# mysql库文件保存路径
MYSQLLIB = -L /usr/local/mysql/lib

# mysql链接库
MYSQLLIBS = -l mysqlclient

# 开发框架mysql的cpp文件名，这里直接包含进来，没有才有链接库，为了方便调试
MYSQLCPP = /project/public/db/mysql/_mysql.cpp

# 编译参数
CFLAGS = -g

all:procctl checkproc gzipfiles deletefiles ftpgetfiles ftpputfiles tcpputfiles fileserver tcpgetfiles execsql dminingmysql xmltodb

procctl:procctl.cpp
	g++ -o procctl procctl.cpp
	cp procctl ../bin/.

checkproc:checkproc.cpp
	g++ $(CFLAGS) -o checkproc checkproc.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp checkproc ../bin/.

gzipfiles:gzipfiles.cpp
	g++ $(CFLAGS) -o gzipfiles gzipfiles.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp gzipfiles ../bin/.

deletefiles:deletefiles.cpp
	g++ $(CFLAGS) -o deletefiles deletefiles.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp deletefiles ../bin/.

ftpgetfiles:ftpgetfiles.cpp
	g++ $(CFLAGS) -o ftpgetfiles ftpgetfiles.cpp $(PUBINCL) $(PUBCPP) /project/public/libftp.a /project/public/_ftp.cpp -lm -lc
	cp ftpgetfiles ../bin/.

ftpputfiles:ftpputfiles.cpp
	g++ $(CFLAGS) -o ftpputfiles ftpputfiles.cpp $(PUBINCL) $(PUBCPP) /project/public/libftp.a /project/public/_ftp.cpp -lm -lc
	cp ftpputfiles ../bin/.

tcpputfiles:tcpputfiles.cpp
	g++ $(CFLAGS) -o tcpputfiles tcpputfiles.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp tcpputfiles ../bin/.

fileserver:fileserver.cpp
	g++ $(CFLAGS) -o fileserver fileserver.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp fileserver ../bin/.

tcpgetfiles:tcpgetfiles.cpp
	g++ $(CFLAGS) -o tcpgetfiles tcpgetfiles.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp tcpgetfiles ../bin/.

execsql:execsql.cpp
	g++ $(CFLAGS) -o execsql execsql.cpp $(PUBINCL) $(PUBCPP) $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) $(MYSQLCPP) -lm -lc
	cp execsql ../bin/.

dminingmysql:dminingmysql.cpp
	g++ $(CFLAGS) -o dminingmysql dminingmysql.cpp $(PUBINCL) $(PUBCPP) $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) $(MYSQLCPP) -lm -lc
	cp dminingmysql ../bin/.

xmltodb:xmltodb.cpp
	g++ $(CFLAGS) -o xmltodb xmltodb.cpp $(PUBINCL) $(PUBCPP) $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) $(MYSQLCPP) -lm -lc
	cp xmltodb ../bin/.

clean:
	rm -f procctl checkproc gzipfiles deletefiles ftpgetfiles ftpputfiles tcpputfiles fileserver tcpgetfiles execsql dminingmysql xmltodb
