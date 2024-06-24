#!/bin/bash

REPORT=./report.info
TMP=${REPORT}.bak

cd ../src

make clean

make cov 

cd -

make clean

make cov

./gtester "$@"

lcov -d ../ -c -o ${TMP}
lcov --remove ${TMP} -o ${REPORT} \
    '/usr/include/*' \
    '/usr/lib/*'

rm -f ${TMP}

genhtml ${REPORT} -o ./result

#if [ -d /home/clouder ]
#then
#    su - clouder xdg-open ./result/index.html
#else
#    xdg-open ./result/index.html
#fi
xdg-open ./result/index.html
