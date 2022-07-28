# mysql头文件存放的目录
MYSQLINCL = -I /usr/local/mysql/include

# mysql库文件存放的目录
MYSQLLIB = -L /usr/local/mysql/lib

# 需要链接的mysql库
MYSQLLIBS = -l mysqlclient

CFLAGS = -g -Wno-write-strings

all:my_createtable my_inserttable my_updatetable my_selecttable my_deletetable my_filetoblob my_blobtofile

my_createtable:my_createtable.cpp _mysql.h _mysql.cpp 
	g++ $(CFLAGS) -o my_createtable my_createtable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

my_inserttable:my_inserttable.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o my_inserttable my_inserttable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

my_updatetable:my_updatetable.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o my_updatetable my_updatetable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

my_selecttable:my_selecttable.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o my_selecttable my_selecttable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

my_deletetable:my_deletetable.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o my_deletetable my_deletetable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

my_filetoblob:my_filetoblob.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o my_filetoblob my_filetoblob.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

my_blobtofile:my_blobtofile.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o my_blobtofile my_blobtofile.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

clean:
	rm -rf my_createtable my_inserttable my_updatetable my_selecttable my_deletetable my_filetoblob my_blobtofile