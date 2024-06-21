#!/bin/bash

cd ../src

make clean

make cov 

cd -

make clean

make cov

./gtester "$@"

lcov -d ../ -c -o ./report.info
genhtml ./report.info -o ./result

#if [ -d /home/clouder ]
#then
#    su - clouder xdg-open ./result/index.html
#else
#    xdg-open ./result/index.html
#fi
xdg-open ./result/index.html
