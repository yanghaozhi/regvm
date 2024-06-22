#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SETS    1   1   123
SETS    2   2   321
SETD    3   3   321.12
SETC    4   4   abc
# watch ：1/2/3/4
TRAP    4   0
CONV    3   1
CONV    1   3
CONV    2   1
TRAP    3   1
STORE   1   4
# $1.from != NULL, $1.ref == 2
# $abc.reg == 1, $abc.ref == 2
TRAP    2   2
CONV    1   1
# $1.from == NULL, $1.ref == -1, $1.type == 1
# $abc.reg == 1, $abc.ref == 2, $abc.type == 3
TRAP    2   3
EXIT    0   0
)";

TEST(code, conv)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 0, 1, EQ, I, 123,    TYPE_SIGNED,   -1);
            CHECK_REG(key, 0, 2, EQ, I, 321,    TYPE_UNSIGNED, -1);
            CHECK_REG(key, 0, 3, EQ, F, 321.12, TYPE_DOUBLE,   -1);
            CHECK_REG(key, 0, 4, EQ, S, "abc",  TYPE_STRING,   -1);
                                          
            CHECK_REG(key, 1, 1, EQ, F, 123.0,  TYPE_DOUBLE,   -1);
            CHECK_REG(key, 1, 2, EQ, I, 321,    TYPE_SIGNED,   -1);
            CHECK_REG(key, 1, 3, EQ, I, 321,    TYPE_SIGNED,   -1);

            CHECK_REG(key, 2, 1, NE, F, 123.0,  TYPE_DOUBLE,  2);

            CHECK_REG(key, 3, 1, EQ, I, 123,    TYPE_SIGNED,  -1);

            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;

            CHECK_VAR(key, 2, "abc", 0, 0, 1,  F, 123.0, TYPE_DOUBLE, 2);

            CHECK_VAR(key, 3, "abc", 0, 0, -1, F, 123.0, TYPE_DOUBLE, 1);
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}


