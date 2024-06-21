#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 12
SETS    1   1   12
TRAP    0   1
# $0 = 0
CLEAR   0   1
TRAP    0   2
# $0 += $1
#LABEL: aaa
ADD     0   1
#TRAP    0   3
# $1 -= 1
DEC     1   1
#TRAP    0   4
# $2 = -6
#SETS    2   8   -7
SETC    2   9   #LABEL:  aaa
#TRAP    0   5
# if ($1 > 0) jump $2
JG      1   2
TRAP    0   6
# $0 += 10
INC     0   10
TRAP    0   7
)";

TEST(mix, loop)
{
    tester t([](auto key, auto offset, auto info)
        {
            bool match = false;
            CHECK_REG(key, 1, 1, EQ, I, 12, TYPE_SIGNED, -1);
            CHECK_REG(key, 2, 0, EQ, I, 0,  TYPE_SIGNED, -1);
            CHECK_REG(key, 6, 0, EQ, I, 78, TYPE_SIGNED, -1);
            CHECK_REG(key, 7, 0, EQ, I, 88, TYPE_SIGNED, -1);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            return true;
        });
    ASSERT_EQ(88, t.go(txt));
}

