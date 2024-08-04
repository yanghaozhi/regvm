#!/bin/bash

REPORT=./report.info
TMP=${REPORT}.bak

cd ../src
make clean
make cov 
cd -

make clean
make cov -j4

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

cloc ../src/core/ ../src/ext/

which xdg-open > /dev/null
if [ $? -eq 0 ]
then
    xdg-open ./result/index.html
fi


