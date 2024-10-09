#!/bin/bash

FILE=pi

declare -A scripts=(["lua"]="lua" ["python3"]="py" ["quickjs"]="js" ["node"]="js")
for k in "${!scripts[@]}"
do
    which $k > /dev/null
    if [ $? -eq 0 ]
    then
        echo
        echo
        echo testing $k ${FILE}.${scripts[$k]} ...
        time $k ${FILE}.${scripts[$k]}
    fi
done


echo
echo
echo test vasm ...
time ../out/vasm ./pi.vasm -r

echo
echo
echo test vim ...
time vim -c "redir! > /dev/stdout" -c "so ./pi.vim" -c "q"
