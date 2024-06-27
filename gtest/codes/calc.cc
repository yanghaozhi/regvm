#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SETS    0   1   456
SETS    1   1   123
SETS    2   2   321
SETD    3   3   321.12
SETC    4   4   abc
# watch ：1/2/3/4
TRAP    5   0

INC     0   5
DEC     3   3
ADD     2   1
# $0 == 456 + 5 == 461, $3 == 321.12 - 3 == 318.12, $2 == 321 + 123 == 444
TRAP    3   1

SUB     1   3
SUB     2   0
# $1 == 123 - 321.12 == -195, $2 == 444 - 461 == -17
TRAP    4   2

DIV     3   1
MUL     1   2
# $3 == 318.12 / -195 == -1.63, $1 == -195 * -17 == -27
TRAP    2   3

TYPE    5   3
# $5 == $0.type
TRAP    1   4

SHR     1   5
SHL     0   5
TRAP    2   5

EXIT    0   0
)";

TEST(code, calc)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 0, 0, N, TYPE_SIGNED,   456,     -1, 0);
            CHECK_REG(key, 0, 1, N, TYPE_SIGNED,   123,     -1, 0);
            CHECK_REG(key, 0, 2, N, TYPE_UNSIGNED, 321,     -1, 0);
            CHECK_REG(key, 0, 3, N, TYPE_DOUBLE,   321.12,  -1, 0);
            CHECK_REG(key, 0, 4, N, TYPE_STRING,   "abc",   -1, 0);
                                                      
            CHECK_REG(key, 1, 0, N, TYPE_SIGNED,   461,     -1, 0);
            CHECK_REG(key, 1, 3, N, TYPE_DOUBLE,   318.12,  -1, 0);
            CHECK_REG(key, 1, 2, N, TYPE_UNSIGNED, 444,     -1, 0);
                                                   
            CHECK_REG(key, 2, 0, N, TYPE_SIGNED,   461,     -1, 0);
            CHECK_REG(key, 2, 1, N, TYPE_SIGNED,   -195,    -1, 0);
            CHECK_REG(key, 2, 2, N, TYPE_UNSIGNED, (uint64_t)-17,     -1, 0);
            CHECK_REG(key, 2, 3, N, TYPE_DOUBLE,   318.12,  -1, 0);

            CHECK_REG(key, 3, 3, N, TYPE_DOUBLE,   318.12 / -195, -1, 0);
            CHECK_REG(key, 3, 1, N, TYPE_SIGNED,   -195 * -17,     -1, 0);

            CHECK_REG(key, 4, 5, N, TYPE_UNSIGNED, TYPE_DOUBLE,   -1, 0);

            CHECK_REG(key, 5, 0, N, TYPE_SIGNED,   3688, -1, 0);
            CHECK_REG(key, 5, 1, N, TYPE_SIGNED,   414,     -1, 0);

            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            //CHECK_VAR(key, 2, "abc", 0, 0, 1,  F, 123.0, TYPE_DOUBLE, 2, 0);

            //CHECK_VAR(key, 3, "abc", 0, 0, -1, F, 123.0, TYPE_DOUBLE, 1, 0);
            return match;
        });
    ASSERT_EQ(0, t.go(txt, true, NULL));
}



