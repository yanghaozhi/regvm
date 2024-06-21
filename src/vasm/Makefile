SRCS = $(wildcard ./*.cpp)
OBJS += $(patsubst %.cpp, %.o, $(SRCS))
LIBS += $(patsubst ./%.cpp, lib%.a, $(shell find -iname '*.cpp' | grep -v main))

ROOT=$(CURDIR)/../../
NAME=vasm
OUT=$(ROOT)/out

DEPS=$(OUT)/libcore.a $(OUT)/libext.a

all: $(NAME)

tt:
	@echo $(SRCS)
	@echo $(LIBS)
	@echo $(OBJS)


INC=-I$(ROOT)/include -I$(ROOT)/src/include
OPTS=-O0 -g -fPIC -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing
CFLAGS=-std=gnu11 $(OPTS) $(INC)
CXXFLAGS=-std=c++17 $(OPTS) $(INC)

CC=gcc
CXX=g++
#CC=clang
#CXX=clang++


clean:
	rm -f $(OUT)/$(NAME) $(OBJS) $(patsubst %.o, %.d, $(OBJS))

lib:$(LIBS)

$(OBJS) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $*.o -MMD $<


$(NAME):$(OBJS) $(DEPS)
	mkdir -p $(OUT)
	ar rcs $(OUT)/lib$(NAME).a  $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUT)/$(NAME) $(OBJS) $(DEPS) -lfmt
	ln -sf $(OUT)/$(NAME) $(NAME)


-include *.d


