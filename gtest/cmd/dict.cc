#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SETS    1   1   123
SETS    2   2   321
SETD    3   3   321.123
SETC    4   4   abc
SETC    5   4   def
SETC    6   4   qwer
# watch ：1/2/3/4
TRAP    6   0

# $7 = dict()
CLEAR   7   5
TRAP    1   1

# $7[$4] = $3
DICT    0   1   7   4   3
TRAP    2   2

# $7[$5] = $2
# $7[$6] = $1
DICT    0   1   7   5   2
DICT    0   1   7   6   1
TRAP    3   3

# $0 = $7[$4]
DICT    0   2   7   4
TRAP    1   4

# $0 = $4 in $7
DICT    0   4   7   4
TRAP    1   5

# $9, $10 = $7.items()
DICT    8   5   7   2   9   10
TRAP    3   6

# del $7[$4]
DICT    0   3   7   4
TRAP    2   7

EXIT    0   0
)";

TEST(cmd, dict)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 0, 1, N, TYPE_SIGNED,    123,        -1, 0);
            CHECK_REG(key, 0, 2, N, TYPE_UNSIGNED,  321,        -1, 0);
            CHECK_REG(key, 0, 3, N, TYPE_DOUBLE,    321.123,    -1, 0);
            CHECK_REG(key, 0, 4, N, TYPE_STRING,    "abc",      -1, 0);
            CHECK_REG(key, 0, 5, N, TYPE_STRING,    "def",      -1, 0);
            CHECK_REG(key, 0, 6, N, TYPE_STRING,    "qwer",     -1, 0);

            CHECK_REG(key, 1, 7, N, TYPE_DICT,      0,          -1, 1);

            CHECK_REG(key, 2, 7, N, TYPE_DICT,      1,          -1, 1);
            CHECK_REG(key, 2, 3, Y, TYPE_DOUBLE,    321.123,    2, 0);

            CHECK_REG(key, 3, 7, N, TYPE_DICT,      3,          -1, 1);
            CHECK_REG(key, 3, 1, Y, TYPE_SIGNED,    123,        2, 0);
            CHECK_REG(key, 3, 2, Y, TYPE_UNSIGNED,  321,        2, 0);

            CHECK_REG(key, 4, 0, Y, TYPE_DOUBLE,    321.123,    2, 0);

            CHECK_REG(key, 5, 0, N, TYPE_SIGNED,    1,          -1, 0);

            CHECK_REG(key, 6, 8, N, TYPE_SIGNED,    3,          -1, 0);
            CHECK_REG(key, 6, 9, N, TYPE_LIST,      3,          -1, 1);
            CHECK_REG(key, 6, 10, N, TYPE_LIST,     3,          -1, 1);

            CHECK_REG(key, 7, 7, N, TYPE_DICT,      2,          -1, 1);
            CHECK_REG(key, 7, 0, N, TYPE_SIGNED,    1,          -1, 0);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}


