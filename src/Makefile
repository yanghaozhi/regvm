SRCS = $(wildcard ./*.cpp)
OBJS += $(patsubst %.cpp, %.o, $(SRCS))
LIBS += $(patsubst ./%.cpp, lib%.a, $(shell find -iname '*.cpp' | grep -v main))

OUT=libregvm.so
DIR=../libs

DEPS= 

all: $(OUT)

tt:
	@echo $(SRCS)
	@echo $(LIBS)
	@echo $(OBJS)


OPTS=-O2 -g -fPIC -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing -I../include -DNDEBUG
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
	mkdir -p $(DIR)
	$(CXX) $(CXXFLAGS) -shared -o $(DIR)/$(OUT) $(OBJS) $(DEPS)


-include *.d


