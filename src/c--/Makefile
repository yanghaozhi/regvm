SRCS = $(wildcard ./*.cpp)
OBJS += $(patsubst %.cpp, %.o, $(SRCS))
LIBS += $(patsubst ./%.cpp, lib%.a, $(shell find -iname '*.cpp' | grep -v main))

ROOT=$(CURDIR)/../../
NAME=vcc
OUT=$(ROOT)/out

EXT=mem

DEPS=$(OUT)/libcore.a $(OUT)/lib$(EXT).a

all: $(NAME)

tt:
	@echo $(SRCS)
	@echo $(LIBS)
	@echo $(OBJS)
	@echo $(LDFLAGS)


INC=-I$(ROOT)/include -I$(ROOT)/src/include -I$(ROOT)/src/core -I$(ROOT)/src/$(EXT)
OPTS=-O3 -g -fPIC -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing -DREGVM_EXT=mem
CFLAGS=-std=gnu11 $(OPTS) $(INC)
CXXFLAGS=-std=c++17 $(OPTS) $(INC)
GCOV=-fprofile-arcs -ftest-coverage

CC=gcc
CXX=g++
#CC=clang
#CXX=clang++


cov: CXXFLAGS=-std=c++17 $(OPTS) $(INC) $(GCOV)
cov: LDFLAGS=-lgcov

cov:$(NAME)

clean:
	rm -f $(OUT)/$(NAME) $(OBJS) $(patsubst %.o, %.d, $(OBJS))

lib:$(LIBS)

$(OBJS) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $*.o -MMD $<


$(NAME):$(OBJS) $(DEPS)
	echo $(LDFLAGS)
	echo $(CXXDFLAGS)
	mkdir -p $(OUT)
	ar rcs $(OUT)/lib$(NAME).a  $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUT)/$(NAME) $(OBJS) $(DEPS) -lfmt $(LDFLAGS)
	ln -sf $(OUT)/$(NAME) $(NAME)


-include *.d


