#!/bin/bash

#cd ../src/
#
#make clean
#CFLAGS="-O0" make

echo lua
time lua ./pi.lua

echo
echo
echo python
time python3 ./pi.py


echo
echo
echo vasm
time ../out/vasm ./pi.vasm -r
