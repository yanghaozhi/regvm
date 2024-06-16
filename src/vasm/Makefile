SRCS = $(wildcard ./*.cpp)
OBJS += $(patsubst %.cpp, %.o, $(SRCS))
LIBS += $(patsubst ./%.cpp, lib%.a, $(shell find -iname '*.cpp' | grep -v main))

ROOT=../../
NAME=vasm
DIR=$(ROOT)/libs

DEPS=$(DIR)/libcore.a $(DIR)/libext.a -lfmt

all: $(NAME)

tt:
	@echo $(SRCS)
	@echo $(LIBS)
	@echo $(OBJS)


INC=-I$(ROOT)/include -I$(ROOT)/src/include
OPTS=-O0 -g -fPIC -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing
CFLAGS=-std=gnu11 $(OPTS) $(INC)
CXXFLAGS=-std=c++20 $(OPTS) $(INC)

CC=gcc
CXX=g++
#CC=clang
#CXX=clang++


clean:
	rm -f *.o test* *.a *.d

lib:$(LIBS)

$(OBJS) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -MMD $<


$(NAME):$(OBJS)
	mkdir -p $(DIR)
	$(CXX) $(CXXFLAGS) -o $(DIR)/$(NAME) $(OBJS) $(DEPS)
	ln -sf $(DIR)/$(NAME) $(NAME)


-include *.d


