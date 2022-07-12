# 开发框架头文件路径
PUBINCL = -I /project/public

# 开发框架cpp文件名，这里直接包含进来，没有才有链接库，是为了方便调试
PUBCPP = /project/public/_public.cpp

# 编译参数
CFLAGS = -g

all:procctl checkproc

procctl:procctl.cpp
	g++ -o procctl procctl.cpp
	cp procctl ../bin/.

checkproc:checkproc.cpp
	g++ $(CFLAGS) -o checkproc checkproc.cpp $(PUBINCL) $(PUBCPP)
	cp checkproc ../bin/.

clean:
	rm -f procctl checkproc
