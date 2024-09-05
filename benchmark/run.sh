#!/bin/bash

FILE=${1:-pi}
LOG=./t.log

cd ../src/

make clean
make release

cd -

declare -A results=()

declare -A vm=(["vasm"]="vasm" ["vcc"]="c--")
for k in "${!vm[@]}"
do
    if [ -f ${FILE}.${vm[${k}]} ]
    then
        echo
        echo
        echo testing ${k} ${FILE}.${vm[${k}]} ...
        (time ../out/${k} ${FILE}.${vm[${k}]} -r) &> ${LOG}
        t=$(cat ${LOG} | grep real | awk '{print $2}' | awk -F[ms] '{print $1 * 60 + $2}')
        results[${k}]=${t}
        cat ${LOG}
        rm -rf ${LOG}
    fi
done

declare -A scripts=(["lua"]="lua" ["python3"]="py" ["qjs"]="js" ["duk"]="js" ["node"]="js")
for k in "${!scripts[@]}"
do
    which ${k} > /dev/null
    if [ $? -eq 0 ] && [ -f ${FILE}.${scripts[${k}]} ]
    then
        echo
        echo
        echo testing ${k} ${FILE}.${scripts[${k}]} ...
        (time ${k} ${FILE}.${scripts[${k}]}) &> ${LOG}
        t=$(cat ${LOG} | grep real | awk '{print $2}' | awk -F[ms] '{print $1 * 60 + $2}')
        cat ${LOG}
        rm -rf ${LOG}

        echo compare time result :
        for r in "${!results[@]}"
        do
            echo "${r} : ${t} / ${results[${r}]} =" $(echo "${t} ${results[${r}]}" | awk '{print $1 / $2}')
        done
    fi
done


