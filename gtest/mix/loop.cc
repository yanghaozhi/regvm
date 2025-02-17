#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 12
SET     1   1   12
TRAP    1   1   0
# $0 = 0
CLEAR   0   1   0
TRAP    2   1   0
# $0 += $1
#LABEL: aaa
ADD     0   0   1
#TRAP    0   3
# $1 -= 1
CALC    1   1   1
#TRAP    0   4
# $2 = 0
CLEAR    2   1   0
#SETC    2   9   #LABEL:  aaa
#TRAP    0   5
# if ($1 > $2) jump $2
JGT     1   2   0   -3
TRAP    6   1   0
# $0 += 10
CALC    0   10  0
TRAP    7   1   0
)";

TEST(mix, loop)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 1, 1, N, TYPE_SIGNED, 12, -1, 0);
            CHECK_REG(key, 2, 0, N, TYPE_SIGNED, 0,  -1, 0);
            CHECK_REG(key, 6, 0, N, TYPE_SIGNED, 78, -1, 0);
            CHECK_REG(key, 7, 0, N, TYPE_SIGNED, 88, -1, 0);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            return 0;
        });
    ASSERT_EQ(88, t.go(txt));
}

