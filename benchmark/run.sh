#!/bin/bash

cd ../src/

make clean
CFLAGS="-O3" make


#time lua ./fib1.lua
#
#time lua ./fib1.lua
