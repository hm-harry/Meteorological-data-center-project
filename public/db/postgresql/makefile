#########################################

# postgresql头文件存放路径
PGINCL = -I/usr/include

# postgresql库文件保存路径
PGLIB = -L/usr/lib64

# postgresql链接库
PGLIBS = -lpq


#CFLAGS = -O2
CFLAGS = -O2 -Wall

#CFLAGS = -g -Wall
CFLAGS = -g -Wno-write-strings


all:	createtable inserttable selecttable counttable updatetable deletetable

createtable: createtable.cpp _postgresql.h _postgresql.cpp
	g++ $(CFLAGS) -o createtable createtable.cpp _postgresql.cpp  $(PGINCL) $(PGLIB) $(PGLIBS) -lm -lc

inserttable: inserttable.cpp _postgresql.h _postgresql.cpp
	g++ $(CFLAGS) -o inserttable inserttable.cpp _postgresql.cpp  $(PGINCL) $(PGLIB) $(PGLIBS) -lm -lc

selecttable: selecttable.cpp _postgresql.h _postgresql.cpp
	g++ $(CFLAGS) -o selecttable selecttable.cpp _postgresql.cpp  $(PGINCL) $(PGLIB) $(PGLIBS) -lm -lc

counttable: counttable.cpp _postgresql.h _postgresql.cpp
	g++ $(CFLAGS) -o counttable counttable.cpp _postgresql.cpp  $(PGINCL) $(PGLIB) $(PGLIBS) -lm -lc

updatetable: updatetable.cpp _postgresql.h _postgresql.cpp
	g++ $(CFLAGS) -o updatetable updatetable.cpp _postgresql.cpp  $(PGINCL) $(PGLIB) $(PGLIBS) -lm -lc

deletetable: deletetable.cpp _postgresql.h _postgresql.cpp
	g++ $(CFLAGS) -o deletetable deletetable.cpp _postgresql.cpp  $(PGINCL) $(PGLIB) $(PGLIBS) -lm -lc

clean:	
	rm -rf createtable inserttable selecttable counttable updatetable deletetable
