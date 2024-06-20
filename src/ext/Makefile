SRCS = $(wildcard ./*.cpp)
OBJS += $(patsubst %.cpp, %.o, $(SRCS))
LIBS += $(patsubst ./%.cpp, lib%.a, $(shell find -iname '*.cpp' | grep -v main))

ROOT=../../
NAME=ext
DIR=$(ROOT)/libs

DEPS= 

all: $(NAME)

tt:
	@echo $(SRCS)
	@echo $(LIBS)
	@echo $(OBJS)


INC=-I$(ROOT)/include -I$(ROOT)/src/include
OPTS=-O0 -g -fPIC -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing -DDEBUG
CFLAGS=-std=gnu11 $(OPTS) $(INC)
CXXFLAGS=-std=c++17 $(OPTS) $(INC)

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
	$(CXX) $(CXXFLAGS) -shared -o $(DIR)/lib$(NAME).so $(OBJS) $(DEPS)
	ar rcs $(DIR)/lib$(NAME).a  $(OBJS)


-include *.d


