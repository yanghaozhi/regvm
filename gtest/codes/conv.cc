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

            CHECK_REG(key, 0, 1, N, TYPE_SIGNED,   123,    -1, 0);
            CHECK_REG(key, 0, 2, N, TYPE_UNSIGNED, 321,    -1, 0);
            CHECK_REG(key, 0, 3, N, TYPE_DOUBLE,   321.12, -1, 0);
            CHECK_REG(key, 0, 4, N, TYPE_STRING,   "abc",  -1, 0);
                                                      
            CHECK_REG(key, 1, 1, N, TYPE_DOUBLE,   123.0,  -1, 0);
            CHECK_REG(key, 1, 2, N, TYPE_SIGNED,   321,    -1, 0);
            CHECK_REG(key, 1, 3, N, TYPE_SIGNED,   321,    -1, 0);
                                                   
            CHECK_REG(key, 2, 1, Y, TYPE_DOUBLE,   123.0,  2, 0);
                                                   
            CHECK_REG(key, 3, 1, N, TYPE_SIGNED,   123,    -1, 0);

            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;

            CHECK_VAR(key, 2, "abc", 0, 0, 1,  TYPE_DOUBLE, 123.0, 2, 0);
                                                           
            CHECK_VAR(key, 3, "abc", 0, 0, -1, TYPE_DOUBLE, 123.0, 1, 0);
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}


