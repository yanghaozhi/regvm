#!/bin/bash

if [ $# -eq 0 ]
then
    echo Usage : $(basename $0) [compile mode] \| [project]
    exit 0
fi

#FILE=${2:-pi}
LOG=./t.log

cd ../src/

make clean
make $1

shift 1

cd -

./test.sh "$@"
