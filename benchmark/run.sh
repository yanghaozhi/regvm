#!/bin/bash

FILE=${1:-pi}
LOG=./t.log

cd ../src/

make clean
make release

cd -

./test.sh "$@"
