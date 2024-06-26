#!/bin/bash

#!/bin/bash

# cppcheck 命令行选项
OPTIONS="--enable=style \
         --enable=performance \
         --enable=portability"

# 根据你的配置，添加相应的检查项
EXTRA_OPTIONS="--enable=unusedFunction"

# 根据你的配置，添加相应的抑制选项
SUPPRESS="--suppress=information \
          --suppress=missingIncludeSystem \
          --suppress=unmatchedSuppression \
          --suppress=passedByValue \
          --suppress=unusedFunction \
          --suppress=uninitMemberVar \
          --suppress=uninitvar \
          --suppress=nullPointer \
          --suppress=shadowVariable \
          --suppress=variableScope \
          --suppress=duplicateExpression \
          --suppress=useInitializationList"

# 包含路径
INCLUDE="-I /usr/include -I/usr/local/include -I../include -I./include"

# 如果需要设置最大函数复杂度，可以取消下面这行的注释
# OPTIONS="$OPTIONS --max-complexity=10"

# 运行 cppcheck
#cppcheck ./ $OPTIONS $EXTRA_OPTIONS $SUPPRESS $INCLUDE
#cppcheck ./ --force $OPTIONS $EXTRA_OPTIONS $INCLUDE
cppcheck ./ --enable=warning,performance,style,unusedFunction --inconclusive --force

