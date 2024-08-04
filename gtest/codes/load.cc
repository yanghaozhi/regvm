#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SET     1   1   123
SET     2   2   321
SET     3   3   321.123
SET     4   4   abc
# watch ：1/2/3/4
TRAP    0   4   0

STORE   1   4   1
# $1.value == 123, $1.ref == 2
# $abc.value == 123, $abc.ref == 2
TRAP    1   1   1

LOAD    6   4   0
# $1.value == 123, $1.ref == -1
# $6.value == 123, $6.ref == 2
# $abc.value == 123, $abc.ref == 2
TRAP    2   2   1

BLOCK   0   0   0

# $4 = "def"
SET     5   4   def
STORE   3   5   1
# $3.ref == 2
# $def.value == 321.123, $def.ref == 2
TRAP    3   1   1

STORE   2   4   1
# $abc.0.reg == 6, $abc.0.type == 2, $abc.0.ref == 2
# $abc.1.value == 321, $abc.1.type == 2, $abc.1.ref == 2
# $2.ref ==  2
# $6.ref ==  2
TRAP    4   2   2

LOAD    7   4   0
# $2.ref == -1
# $7.value == 321, $7.ref == 2
# $abc.0.ref == 2
# $abc.1.ref == 2
TRAP    5   2   2

BLOCK   0   1   0

# $7.value == 321, $7.ref == -1
# $3.value == 321.123, $7.ref == -1
# $abc.0.reg == 6, $abc.0.type == 2, $abc.0.ref == 2
TRAP    6   2   1

# $abc.1 == NULL
# $def.1 == NULL
TRAP    7   0   0

LOAD    8   4   0
# $6.value == 123, $6.ref == -1
# $8.value == 123, $8.ref == 2
# $abc.value == 123, $abc.ref == 2
TRAP    8   2   1

EXIT    0   0   0
)";

TEST(code, load)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 0, 1, N, TYPE_SIGNED,    123,     -1, 0);
            CHECK_REG(key, 0, 2, N, TYPE_UNSIGNED,  321,     -1, 0);
            CHECK_REG(key, 0, 3, N, TYPE_DOUBLE,    321.123, -1, 0);
            CHECK_REG(key, 0, 4, N, TYPE_STRING,    "abc",   -1, 0);
                                                       
            CHECK_REG(key, 1, 1, Y, TYPE_SIGNED,    123,     2, 0);
                                                    
            CHECK_REG(key, 2, 1, N, TYPE_SIGNED,    123,     -1, 0);
            CHECK_REG(key, 2, 6, Y, TYPE_SIGNED,    123,     2, 0);

            CHECK_REG(key, 3, 3, Y, TYPE_DOUBLE,    321.123, 2, 0);

            CHECK_REG(key, 4, 2, Y, TYPE_UNSIGNED,  321,     2, 0);
            CHECK_REG(key, 4, 6, Y, TYPE_SIGNED,    123,     2, 0);

            CHECK_REG(key, 5, 2, N, TYPE_UNSIGNED,  321,     -1, 0);
            CHECK_REG(key, 5, 7, Y, TYPE_UNSIGNED,  321,     2, 0);

            CHECK_REG(key, 6, 3, Y, TYPE_DOUBLE,    321.123, 1, 0);
            CHECK_REG(key, 6, 7, Y, TYPE_UNSIGNED,  321,     1, 0);

            CHECK_REG(key, 8, 6, N, TYPE_SIGNED,    123,    -1, 0);
            CHECK_REG(key, 8, 8, Y, TYPE_SIGNED,    123,    2, 0);

            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;

            CHECK_VAR(key, 1, "abc", 0, 0, 1,   TYPE_SIGNED,    123, 2, 0);

            CHECK_VAR(key, 2, "abc", 0, 0, 6,   TYPE_SIGNED,    123, 2, 0);

            CHECK_VAR(key, 3, "def", 0, 1, 3,   TYPE_DOUBLE,    321.123, 2, 0);

            CHECK_VAR(key, 4, "abc", 0, 0, 6,   TYPE_SIGNED,    123, 2, 0);
            CHECK_VAR(key, 4, "abc", 0, 1, 2,   TYPE_UNSIGNED,  321, 2, 0);

            CHECK_VAR(key, 5, "abc", 0, 0, 6,   TYPE_SIGNED,    123, 2, 0);
            CHECK_VAR(key, 5, "abc", 0, 1, 7,   TYPE_UNSIGNED,  321, 2, 0);

            CHECK_VAR(key, 6, "abc", 0, 0, 6,   TYPE_SIGNED,    123, 2, 0);
            CHECK_VAR(key, 6, "abc", 0, 1, 7,   TYPE_UNSIGNED,  321, 2, 0);

            CHECK_VAR(key, 7, "abc", 0, 1, 7,   TYPE_UNSIGNED,  321, 2, 0);
            CHECK_VAR(key, 7, "def", 0, 1, 7,   TYPE_DOUBLE,    321.123, 2, 0);

            CHECK_VAR(key, 8, "abc", 0, 0, 8,   TYPE_SIGNED,    123, 2, 0);
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}

