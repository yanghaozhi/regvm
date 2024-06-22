#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 12
SETS    0   1   12
SETS    1   1   321
SETC    2   9   #LABEL: test
TRAP    2   1
#call test
CALL    2   2
#continue
TRAP    1   2
SETD    1   3   3.21
TRAP    1   6
ADD     0   1
TRAP    1   7
#exit
EXIT    0   1
#function test
#LABEL: test
TRAP    2   3
CONV    0   3
CONV    1   3
TRAP    2   4
# $1 /= $0
DIV     1   0
# $0 = $1
MOVE    0   1
TRAP    2   5
# ret $0
RET     0   1
)";

TEST(mix, call)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key,  1, 0, EQ, I, 12,    TYPE_SIGNED, -1);
            CHECK_REG(key,  1, 1, EQ, I, 321,   TYPE_SIGNED, -1);

            CHECK_REG(key,  2, 0, EQ, F, 26.75, TYPE_DOUBLE, -1);

            CHECK_REG(key,  3, 0, EQ, I, 12,    TYPE_SIGNED, -1);
            CHECK_REG(key,  3, 1, EQ, I, 321,   TYPE_SIGNED, -1);

            CHECK_REG(key,  4, 0, EQ, F, 12,    TYPE_DOUBLE, -1);
            CHECK_REG(key,  4, 1, EQ, F, 321,   TYPE_DOUBLE, -1);

            CHECK_REG(key,  5, 0, EQ, F, 26.75, TYPE_DOUBLE, -1);
            CHECK_REG(key,  5, 1, EQ, F, 26.75, TYPE_DOUBLE, -1);

            CHECK_REG(key,  6, 1, EQ, F, 3.21, TYPE_DOUBLE, -1);

            CHECK_REG(key,  7, 0, EQ, F, 29.96, TYPE_DOUBLE, -1);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            return 0;
        });
    ASSERT_EQ(29, t.go(txt));
}

