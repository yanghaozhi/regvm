SRCS = $(wildcard ./*.cpp)
OBJS += $(patsubst %.cpp, %.o, $(SRCS))
LIBS += $(patsubst ./%.cpp, lib%.a, $(shell find -iname '*.cpp' | grep -v main))

OUT=tester
DIR=../libs

DEPS= 

all: $(OUT)

tt:
	@echo $(SRCS)
	@echo $(LIBS)
	@echo $(OBJS)


OPTS=-O0 -g -fPIC -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing -I../include -DDEBUG
CFLAGS=-std=gnu11 $(OPTS)
CXXFLAGS=-std=c++17 $(OPTS)

CC=gcc
CXX=g++
#CC=clang
#CXX=clang++


clean:
	rm -f *.o test* *.a *.d

lib:$(LIBS)

$(OBJS) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -MMD $<


$(OUT):$(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUT) $(OBJS) $(DEPS) $(DIR)/libregvm.so


-include *.d


