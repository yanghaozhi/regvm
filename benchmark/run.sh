#!/bin/bash

FILE=${1:-pi}

cd ../src/

make clean
make release

cd -

declare -A scripts=(["lua"]="lua" ["python3"]="py" ["qjs"]="js" ["duk"]="js" ["node"]="js")
for k in "${!scripts[@]}"
do
    which $k > /dev/null
    if [ $? -eq 0 ] && [ -f ${FILE}.${scripts[$k]} ]
    then
        echo
        echo
        echo testing $k ${FILE}.${scripts[$k]} ...
        time $k ${FILE}.${scripts[$k]}
    fi
done


declare -A vm=(["vasm"]="vasm" ["vcc"]="c--")
for k in "${!vm[@]}"
do
    if [ -f ${FILE}.${vm[$k]} ]
    then
        echo
        echo
        echo testing $k ${FILE}.${vm[$k]} ...
        time ../out/$k ${FILE}.${vm[$k]} -r
    fi
done

