#!/bin/bash

FILE="pi"
CSV=""
LOOP=10
KEY=vcc
VALUE=""
NO_REBUILD=0
LOG=./t.log


while getopts "f:o:l:k:v:r" arg
do
    case $arg in
        f)
            FILE=${OPTARG}
            ;;
        o)
            CSV=${OPTARG}
            ;;
        l)
            LOOP=${OPTARG}
            ;;
        k)
            KEY=${OPTARG}
            ;;
        v)
            VALUE=${OPTARG}
            ;;
        r)
            NO_REBUILD=1
            ;;
        *)
            ;;
    esac
done


declare -A vm=(["vasm"]="vasm" ["vcc"]="c--")
declare -A scripts=(["lua"]="lua" ["python3"]="py" ["qjs"]="js" ["duk"]="js" ["node"]="js")

if [ ${#CSV} -eq 0 ]
then
    CSV=${FILE}.csv
fi

if [ ${#VALUE} -eq 0 ]
then
    VALUES=(${!scripts[@]})
else
    VALUES=(${VALUE//,/ })
fi


if [ ${NO_REBUILD} -eq 0 ]
then
    cd ../src/
    make clean
    make release
    cd -
fi


rm -f ${CSV}

for v in "${!VALUES[@]}"
do
    k=${VALUES[${v}]}
    which ${k} > /dev/null
    if [ $? -eq 0 ] && [ -f ${FILE}.${scripts[${k}]} ]
    then
        echo -n "${k}," >> ${CSV}
    fi
done
echo "" >> ${CSV}


for ((i = 0; i < ${LOOP}; i++))
do
    echo
    echo round ${i} of ${LOOP} ...
    echo

    declare -A results=()
    if [ -f ${FILE}.${vm[${KEY}]} ]
    then
        echo testing ${KEY} ${FILE}.${vm[${KEY}]} ...
        (time ../out/${KEY} ${FILE}.${vm[${KEY}]} -r) &> ${LOG}
        t=$(cat ${LOG} | grep real | awk '{print $2}' | awk -F[ms] '{print $1 * 60 + $2}')
        results[${KEY}]=${t}
        rm -rf ${LOG}
    fi

    for v in "${!VALUES[@]}"
    do
        k=${VALUES[${v}]}
        which ${k} > /dev/null
        if [ $? -eq 0 ] && [ -f ${FILE}.${scripts[${k}]} ]
        then
            echo testing ${k} ${FILE}.${scripts[${k}]} ...
            (time ${k} ${FILE}.${scripts[${k}]}) &> ${LOG}
            t=$(cat ${LOG} | grep real | awk '{print $2}' | awk -F[ms] '{print $1 * 60 + $2}')
            rm -rf ${LOG}

            for r in "${!results[@]}"
            do
                echo "${r} : ${t} / ${results[${r}]} =" $(echo "${t} ${results[${r}]}" | awk '{print $1 / $2}')
            done

            echo -n $(echo "${t} ${results[${r}]}" | awk '{print $1 / $2}'), >> ${CSV}
        fi
    done
    echo "" >> ${CSV}
done

echo
echo
cat ${CSV}

