#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SET     0   1   456
SET     1   1   123
SET     2   2   321
SET     3   3   321.12
SET     4   4   abc
# watch ：1/2/3/4
TRAP    0   5   0

CALC    0   5   0
CALC    3   3   1
ADD     2   2   1
# $0 == 456 + 5 == 461, $3 == 321.12 - 3 == 318.12, $2 == 321 + 123 == 444
TRAP    1   3   0

SUB     1   1   3
SUB     2   2   0
# $1 == 123 - 318.12 == -195.12, $2 == 444 - 461 == -17
TRAP    2   4   0

DIV     8   3   1
MUL     9   1   0
# $8 == 318.12 / -195.12, $9 == -195.12 * 461
TRAP    3   5   0

TYPE    5   3   0
# $5 == $0.type
TRAP    4   1   0

CALC    5   2   5
CALC    0   2   6
TRAP    5   2   0

EXIT    0   0   0
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
            CHECK_REG(key, 2, 1, N, TYPE_DOUBLE,   -195.12, -1, 0);
            CHECK_REG(key, 2, 2, N, TYPE_UNSIGNED, (uint64_t)-17,       -1, 0);
            CHECK_REG(key, 2, 3, N, TYPE_DOUBLE,   318.12,  -1, 0);

            CHECK_REG(key, 3, 0, N, TYPE_SIGNED,   461,     -1, 0);
            CHECK_REG(key, 3, 1, N, TYPE_DOUBLE,   -195.12, -1, 0);
            CHECK_REG(key, 3, 3, N, TYPE_DOUBLE,   318.12,  -1, 0);
            CHECK_REG(key, 3, 8, N, TYPE_DOUBLE,   318.12 / -195.12,    -1, 0);
            CHECK_REG(key, 3, 9, N, TYPE_DOUBLE,   -195.12 * 461,       -1, 0);

            CHECK_REG(key, 4, 5, N, TYPE_SIGNED,   TYPE_DOUBLE,         -1, 0);

            CHECK_REG(key, 5, 0, N, TYPE_SIGNED,   115,     -1, 0);
            CHECK_REG(key, 5, 5, N, TYPE_SIGNED,   12,      -1, 0);

            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            //CHECK_VAR(key, 2, "abc", 0, 0, 1,  F, 123.0, TYPE_DOUBLE, 2, 0);

            //CHECK_VAR(key, 3, "abc", 0, 0, -1, F, 123.0, TYPE_DOUBLE, 1, 0);
            return match;
        });
    ASSERT_EQ(0, t.go(txt, true));
}



