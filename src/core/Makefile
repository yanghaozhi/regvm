SRCS = $(wildcard ./*.cpp)
OBJS += $(patsubst %.cpp, %.o, $(SRCS))
LIBS += $(patsubst ./%.cpp, lib%.a, $(shell find -iname '*.cpp' | grep -v main))

ROOT=$(CURDIR)/../../
NAME=core
OUT=$(ROOT)/out

EXT=mem

DEPS= 

all: $(NAME)

tt:
	@echo $(SRCS)
	@echo $(LIBS)
	@echo $(OBJS)

INC=-I./ -I$(ROOT)/include -I$(ROOT)/src/include -I$(ROOT)/src/$(EXT)
OPTS=-O0 -g -fPIC -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing -DDEBUG -DREGVM_EXT=mem -DLOG_LEVEL=10
CFLAGS=-std=gnu11 $(OPTS) $(INC)
CXXFLAGS=-std=c++17 $(OPTS) $(INC)
GCOV=-fprofile-arcs -ftest-coverage

CC=gcc
CXX=g++
#CC=clang
#CXX=clang++

cov: CXXFLAGS=-std=c++17 $(OPTS) $(INC) $(GCOV)

cov:$(NAME)

clean:
	rm -f $(OUT)/$(NAME) $(OBJS) $(patsubst %.o, %.d, $(OBJS))

lib:$(LIBS)

$(OBJS) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -MMD $<


$(NAME):$(OBJS)
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) -shared -o $(OUT)/lib$(NAME).so $(OBJS) $(DEPS)
	ar rcs $(OUT)/lib$(NAME).a  $(OBJS) $(DEPS)


-include *.d


