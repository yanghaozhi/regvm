#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SETS    1   1   123
SETS    2   2   321
SETD    3   3   321.123
SETC    4   4   abc
# watch ：1/2/3/4
TRAP    4   0

CLEAR   5   6
TRAP    1   1

LIST    0   2   5   0   1
TRAP    1   2

LIST    0   0   5
SETS    6   1   0
TRAP    2   3

LIST    0   2   5   0   3
LIST    0   1   5   6
TRAP    1   4



EXIT    0   0
)";

TEST(cmd, list)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 0, 1, N, TYPE_SIGNED,    123,        -1, 0);
            CHECK_REG(key, 0, 2, N, TYPE_UNSIGNED,  321,        -1, 0);
            CHECK_REG(key, 0, 3, N, TYPE_DOUBLE,    321.123,    -1, 0);
            CHECK_REG(key, 0, 4, N, TYPE_STRING,    "abc",      -1, 0);

            CHECK_REG(key, 1, 5, N, TYPE_LIST,      0,          -1, 1);

            CHECK_REG(key, 2, 5, N, TYPE_LIST,      1,          -1, 1);

            CHECK_REG(key, 3, 0, N, TYPE_SIGNED,    1,          -1, 0);
            CHECK_REG(key, 3, 6, N, TYPE_SIGNED,    0,          -1, 0);

            CHECK_REG(key, 4, 0, Y, TYPE_SIGNED,    123,        2, 0);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}

