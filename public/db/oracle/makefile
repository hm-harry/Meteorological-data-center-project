#########################################
# 请注意，编译时，确定以下几点：
# 1、已安装oracle的客户端或oracle服务端程序，并且必须安装好OCI软件包
# 2、设置好ORACLE_HOME环境变量
#
#########################################

# oracle头文件路径
ORAINCL = -I$(ORACLE_HOME)/rdbms/public 

# oracle库文件路径
ORALIB =  -L$(ORACLE_HOME)/lib -L.

# oracle的oci库
ORALIBS = -lclntsh 

#CFLAGS = -O2
CFLAGS = -O2 -Wall

CFLAGS = -g -Wall -Wno-write-strings #-Wno-unused-variable

all:	createtable inserttable selecttable counttable updatetable deletetable execplsql filetoclob clobtofile filetoblob blobtofile

createtable: createtable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o createtable createtable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc

inserttable: inserttable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o inserttable inserttable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc

selecttable: selecttable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o selecttable selecttable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc

counttable: counttable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o counttable counttable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc

updatetable: updatetable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o updatetable updatetable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc

deletetable: deletetable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o deletetable deletetable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc

execplsql: execplsql.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o execplsql execplsql.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc

filetoclob: filetoclob.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o filetoclob filetoclob.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc

clobtofile: clobtofile.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o clobtofile clobtofile.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc

filetoblob: filetoblob.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o filetoblob filetoblob.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc

blobtofile: blobtofile.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o blobtofile blobtofile.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp -lm -lc


clean:	
	rm -rf createtable inserttable selecttable counttable updatetable deletetable execplsql filetoclob clobtofile filetoblob blobtofile
	rm -rf memo_out.txt  pic_out.jpg
