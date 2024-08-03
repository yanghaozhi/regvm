SRCS = $(wildcard ./*.cpp)
OBJS += $(patsubst %.cpp, %.o, $(SRCS))
LIBS += $(patsubst ./%.cpp, lib%.a, $(shell find -iname '*.cpp' | grep -v main))

ROOT=$(CURDIR)/../../
NAME=vcc
OUT=$(ROOT)/out

EXT=mem

DEPS=$(OUT)/libcore.a $(OUT)/lib$(EXT).a

all: EXTRA := -O0 -g -DLOG_LEVEL=10 -DDEBUG

all: $(NAME)

release: EXTRA := -O3

release: $(NAME)

cov: EXTRA := -O0 -g -fprofile-arcs -ftest-coverage -DLOG_LEVEL=2
cov: LDFLAGS=-lgcov

cov:$(NAME)

tt:
	@echo $(SRCS)
	@echo $(LIBS)
	@echo $(OBJS)
	@echo $(LDFLAGS)


INC=-I$(ROOT)/include -I$(ROOT)/src/include -I$(ROOT)/src/core -I$(ROOT)/src/$(EXT)
OPTS=-fPIC -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing -DREGVM_EXT=$(EXT)
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
	$(CXX) $(EXTRA) $(CXXFLAGS) -c -o $*.o -MMD $<


$(NAME):$(OBJS) $(DEPS)
	echo $(LDFLAGS)
	echo $(CXXDFLAGS)
	mkdir -p $(OUT)
	ar rcs $(OUT)/lib$(NAME).a  $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUT)/$(NAME) $(OBJS) $(DEPS) $(LDFLAGS)
	ln -sf $(OUT)/$(NAME) $(NAME)


-include *.d


