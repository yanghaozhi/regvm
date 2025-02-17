#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SET     1   1   123
SET     2   2   321
SET     3   3   321.123
SET     4   4   abc
SET     5   4   def
SET     6   4   qwer
# watch ：1/2/3/4
TRAP    0   6   0

# $7 = dict()
CLEAR   7   5   0
TRAP    1   1   0

# $7[$4] = $3
DSET    7   4   3
TRAP    2   2   0

# $7[$5] = $2
# $7[$6] = $1
DSET    7   5   2
DSET    7   6   1
TRAP    3   3   0

# $0 = $7[$4]
DGET    0   7   4
TRAP    4   1   0

# $0 = $4 in $7
DHAS    0   7   4
TRAP    5   1   0

# $9, $10 = $7.items()
DITEMS  7   9   10
TRAP    6   2   0

# del $7[$4]
DDEL    7   4   0
TRAP    7   2   0

DLEN    0   7   0
TRAP    8   1   0

EXIT    0   0   0
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

            CHECK_REG(key, 5, 0, Y, TYPE_SIGNED,    1,          2, 0);

            CHECK_REG(key, 6, 9, N, TYPE_LIST,      3,          -1, 1);
            CHECK_REG(key, 6, 10, N, TYPE_LIST,     3,          -1, 1);

            CHECK_REG(key, 7, 7, N, TYPE_DICT,      2,          -1, 1);
            CHECK_REG(key, 7, 0, Y, TYPE_SIGNED,    1,          2, 0);

            CHECK_REG(key, 8, 0, Y, TYPE_SIGNED,    2,          2, 0);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}


