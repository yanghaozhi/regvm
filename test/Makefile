SRCS = $(wildcard ./*.c)
OBJS += $(patsubst %.c, %.o, $(SRCS))
LIBS += $(patsubst ./%.c, lib%.a, $(shell find -iname '*.c' | grep -v main))

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

$(OBJS) : %.o : %.c
	$(CC) $(CFLAGS) -c -MMD $<


$(OUT):$(OBJS)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS) $(DEPS) $(DIR)/libregvm.so


-include *.d


