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

# $5 = list()
CLEAR   5   6
TRAP    1   1

# $5.push_back($1)  [123]
LIST    0   2   5   0   1
# $5.push_front($1) [321, 123]
LIST    0   2   5   1   2
# $5.push_back($3)  [321, 123, 321.123]
LIST    0   2   5   0   3
TRAP    1   2

# $0 = $5.len()
LIST    0   0   5
TRAP    1   3

# $6 = $5[0]
CLEAR   0   1
LIST    6   1   5   0
# $7 = $5[1]
INC     0   1
LIST    7   1   5   0
# $8 = $5[2]
INC     0   1
LIST    8   1   5   0
TRAP    3   4

SETS    9   1   1
# $5.insert($9, $4) [321, "abc", 123, 321.123]
LIST    0   4   5   9   4
TRAP    3   5

# $6 = $5[0]
CLEAR   0   1
LIST    6   1   5   0
# $7 = $5[1]
INC     0   1
LIST    7   1   5   0
# $8 = $5[2]
INC     0   1
LIST    8   1   5   0
TRAP    3   6

# $5[$0] = $9       [321, "abc", 1, 321.123]
LIST    8   6   5   0   9
TRAP    3   7

# $5.erase($0)      [321, "abc", 321.123]
LIST    9   5   5   0
LIST    6   1   5   0
TRAP    3   8

# $6 = $5[0]
CLEAR   0   1
LIST    6   1   5   0
# $7 = $5[1]
INC     0   1
LIST    7   1   5   0
# $8 = $5[2]
INC     0   1
LIST    8   1   5   0
TRAP    3   9

# $6 = $5.pop_back()
LIST    6   3   5   0
# $7 = $5.pop_front()
LIST    7   3   5   1
TRAP    3   10

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

            CHECK_REG(key, 2, 5, N, TYPE_LIST,      3,          -1, 1);

            CHECK_REG(key, 3, 0, N, TYPE_SIGNED,    3,          -1, 0);

            CHECK_REG(key, 4, 6, Y, TYPE_UNSIGNED,  321,        2, 0);
            CHECK_REG(key, 4, 7, Y, TYPE_SIGNED,    123,        2, 0);
            CHECK_REG(key, 4, 8, Y, TYPE_DOUBLE,    321.123,    2, 0);

            CHECK_REG(key, 5, 0, N, TYPE_SIGNED,    1,          -1, 0);
            CHECK_REG(key, 5, 9, N, TYPE_SIGNED,    1,          -1, 0);
            CHECK_REG(key, 5, 5, N, TYPE_LIST,      4,          -1, 1);

            CHECK_REG(key, 6, 6, Y, TYPE_UNSIGNED,  321,        2, 0);
            CHECK_REG(key, 6, 7, Y, TYPE_STRING,    "abc",      2, 0);
            CHECK_REG(key, 6, 8, Y, TYPE_SIGNED,    123,          2, 0);

            CHECK_REG(key, 7, 0, N, TYPE_SIGNED,    2,          -1, 0);
            CHECK_REG(key, 7, 8, N, TYPE_SIGNED,    1,          -1, 0);
            CHECK_REG(key, 7, 9, N, TYPE_SIGNED,    1,          -1, 0);

            CHECK_REG(key, 8, 5, N, TYPE_LIST,      3,          -1, 1);
            CHECK_REG(key, 8, 6, Y, TYPE_DOUBLE,    321.123,    2, 0);
            CHECK_REG(key, 8, 9, N, TYPE_SIGNED,    1,          -1, 0);

            CHECK_REG(key, 9, 6, Y, TYPE_UNSIGNED,  321,        2, 0);
            CHECK_REG(key, 9, 7, Y, TYPE_STRING,    "abc",      2, 0);
            CHECK_REG(key, 9, 8, Y, TYPE_DOUBLE,    321.123,    2, 0);

            CHECK_REG(key, 10, 5, N, TYPE_LIST,     1,    -1, 1);
            CHECK_REG(key, 10, 6, Y, TYPE_DOUBLE,   321.123,   1, 0);
            CHECK_REG(key, 10, 7, Y, TYPE_UNSIGNED, 321,      1, 0);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}

