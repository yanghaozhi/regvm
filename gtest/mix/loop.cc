#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 12
SETS    1   1   12
TRAP    1   1
# $0 = 0
CLEAR   0   1
TRAP    0   1
# $0 += $1
#LABEL: aaa
ADD     0   1
TRAP    0   1
# $1 -= 1
DEC     1   1
TRAP    1   1
# $2 = -6
#SETS    2   8   -7
SETC    2   9   #LABEL:  aaa
TRAP    15   1
# if ($1 > 0) jump $2
JG      1   2
TRAP    0   1
# $0 += 10
INC     0   10
TRAP    0   1
)";

TEST(mix, loop)
{
    tester t([](auto key, auto offset, auto info)
        {
            bool match = false;
            CHECK_REG(offset,  2, 1, EQ, I, 12, 1, -1);
            CHECK_REG(offset,  4, 0, EQ, I, 0,  1, -1);
            CHECK_REG(offset, 16, 0, EQ, I, 78, 1, -1);
            CHECK_REG(offset, 18, 0, EQ, I, 88, 1, -1);
            return true;
        },
        [](auto key, auto offset, auto info)
        {
            return true;
        });
    ASSERT_EQ(88, t.go(txt));
}

