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

CHG     1   1
CHG     3   1
CHG     0   2
CHG     2   3
TRAP    4   1

CHG     2   0
TRAP    1   2

EXIT    0   0
)";

TEST(code, chg)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 0, 0, N, TYPE_SIGNED,   456,     -1);
            CHECK_REG(key, 0, 1, N, TYPE_SIGNED,   123,     -1);
            CHECK_REG(key, 0, 2, N, TYPE_UNSIGNED, 321,     -1);
            CHECK_REG(key, 0, 3, N, TYPE_DOUBLE,   321.12,  -1);
            CHECK_REG(key, 0, 4, N, TYPE_STRING,   "abc",   -1);
                                                      
            CHECK_REG(key, 1, 0, N, TYPE_DOUBLE,   (double)1 / 456,     -1);
            CHECK_REG(key, 1, 1, N, TYPE_SIGNED,   -123,     -1);
            CHECK_REG(key, 1, 2, N, TYPE_UNSIGNED, (uint64_t)-322,     -1);
            CHECK_REG(key, 1, 3, N, TYPE_DOUBLE,   -321.12,  -1);

            CHECK_REG(key, 2, 2, N, TYPE_UNSIGNED, 0,     -1);

            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}



