SRCS = $(wildcard ./vcore/*.cc)
#SRCS = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.cpp))
OBJS += $(patsubst %.cc, %.o, $(SRCS))
LIBS += $(patsubst ./%.cc, lib%.a, $(shell find -iname '*.cc' | grep -v main))

ROOT=$(CURDIR)/../
NAME=gtester
OUT=$(ROOT)/libs

DEPS= $(OUT)/libvasm.a $(OUT)/libext.a $(OUT)/libcore.a /usr/lib/x86_64-linux-gnu/libgtest.a -lpthread

all: $(NAME)

tt:
	@echo $(SRCS)
	@echo $(LIBS)
	@echo $(OBJS)
	@echo $(SRCS:.cc=.o)


INC=-I$(ROOT)/include -I$(ROOT)/src/include -I$(ROOT)/src/vasm
OPTS=-O0 -g -fPIC -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing
CFLAGS=-std=gnu11 $(OPTS) $(INC)
CXXFLAGS=-std=c++17 $(OPTS) $(INC)

CC=gcc
CXX=g++
#CC=clang
#CXX=clang++


clean:
	rm -f *.o test* *.a *.d $(OUT)/$(NAME) $(OBJS) $(patsubst %.cc, %.d, $(SRCS))

lib:$(LIBS)

$(OBJS) : %.o : %.cc
	$(CXX) $(CXXFLAGS) -c -o $*.o -MMD $<


$(NAME):$(OBJS) $(DEPS) 
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) $(OBJS) main.cc -o $(OUT)/$(NAME) $(DEPS) -lfmt
	ln -sf $(OUT)/$(NAME) $(NAME)


-include *.d


