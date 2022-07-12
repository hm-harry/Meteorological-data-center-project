# 开发框架头文件路径
PUBINCL = -I /project/public

# 开发框架cpp文件名，这里直接包含进来，没有采用链接库，是为了方便调试
PUBCPP = /project/public/_public.cpp

# 编译参数
CFLAGS = -g

all:crtsurfdata1 crtsurfdata2 crtsurfdata3 crtsurfdata4 crtsurfdata5 crtsurfdata

crtsurfdata1:crtsurfdata1.cpp
	g++ $(CFLAGS) -o crtsurfdata1 crtsurfdata1.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp crtsurfdata1 ../bin

crtsurfdata2:crtsurfdata2.cpp
	g++ $(CFLAGS) -o crtsurfdata2 crtsurfdata2.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp crtsurfdata2 ../bin

crtsurfdata3:crtsurfdata3.cpp
	g++ $(CFLAGS) -o crtsurfdata3 crtsurfdata3.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp crtsurfdata3 ../bin

crtsurfdata4:crtsurfdata4.cpp
	g++ $(CFLAGS) -o crtsurfdata4 crtsurfdata4.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp crtsurfdata4 ../bin

crtsurfdata5:crtsurfdata5.cpp
	g++ $(CFLAGS) -o crtsurfdata5 crtsurfdata5.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp crtsurfdata5 ../bin

crtsurfdata:crtsurfdata.cpp
	g++ $(CFLAGS) -o crtsurfdata crtsurfdata.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp crtsurfdata ../bin

clean:
	rm crtsurfdata1 crtsurfdata2 crtsurfdata3 crtsurfdata4 crtsurfdata5 crtsurfdata
