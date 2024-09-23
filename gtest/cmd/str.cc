#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SET     1   1   123
SET     2   2   321
SET     3   7   111
SET     4   4   abc
# watch ：1/2/3/4
TRAP    1   4   0

SLEN    0   4   0
# $0 = strlen("abc")
TRAP    2   1   0

ECHO    2   0   4
CHG     9   4   3
TRAP    3   1   0

BLOCK   0   0   0

SET     4   4   asdf
SET     5   4   def
SET     6   4   qwerzxcv
TRAP    4   3   0

STORE   5   3   1
TRAP    5   2   1

STORE   6   3   1
TRAP    6   2   1

BLOCK   0   1   0

TRAP    7   2   0

#SETS    1   1   2
#SETS    2   1   3
#STR     7   1   6   1   2
#TRAP    4   8

EXIT    0   0   0
)";

TEST(cmd, str)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 1, 1, N, TYPE_SIGNED,    123,    -1, 0);
            CHECK_REG(key, 1, 2, N, TYPE_UNSIGNED,  321,    -1, 0);
            CHECK_REG(key, 1, 3, N, TYPE_ADDR,      111,    -1, 0);
            CHECK_REG(key, 1, 4, N, TYPE_STRING,    "abc",  -1, 0);

            CHECK_REG(key, 2, 0, N, TYPE_SIGNED,    3,   -1, 0);

            CHECK_REG(key, 3, 9, N, TYPE_STRING,    "abc",   -1, 1);

            CHECK_REG(key, 4, 4, N, TYPE_STRING,    "asdf",   -1, 0);
            CHECK_REG(key, 4, 5, N, TYPE_STRING,    "def",   -1, 0);
            CHECK_REG(key, 4, 6, N, TYPE_STRING,    "qwerzxcv",   -1, 0);

            CHECK_REG(key, 5, 4, N, TYPE_STRING,    "asdf",   -1, 0);
            CHECK_REG(key, 5, 5, Y, TYPE_STRING,    "def",   2, 0);

            CHECK_REG(key, 6, 5, N, TYPE_STRING,    "def",   -1, 0);
            CHECK_REG(key, 6, 6, Y, TYPE_STRING,    "qwerzxcv",   2, 0);

            CHECK_REG(key, 7, 5, N, TYPE_STRING,    "def",   -1, 0);
            CHECK_REG(key, 7, 6, Y, TYPE_STRING,    "qwerzxcv",   2, 0);

            CHECK_REG(key, 8, 1, N, TYPE_SIGNED,    2,   -1, 0);
            CHECK_REG(key, 8, 2, N, TYPE_SIGNED,    3,   -1, 0);
            CHECK_REG(key, 8, 6, Y, TYPE_STRING,    "qwerzxcv",   1, 0);
            CHECK_REG(key, 8, 7, N, TYPE_STRING,    "erz",   -1, 1);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_VAR(key, 5, 111, 0, 5,  TYPE_STRING, "def", 2, 0);

            CHECK_VAR(key, 6, 111, 0, 6,  TYPE_STRING, "qwerzxcv", 2, 0);
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}

