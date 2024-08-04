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

CHG     6   0   1
CHG     7   1   0
CHG     8   2   2
CHG     9   3   0
TRAP    1   8   0

CHG     5   4   3
TRAP    2   1   0

EXIT    0   0   0
)";

TEST(code, chg)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 0, 0, N, TYPE_SIGNED,   456,     -1, 0);
            CHECK_REG(key, 0, 1, N, TYPE_SIGNED,   123,     -1, 0);
            CHECK_REG(key, 0, 2, N, TYPE_UNSIGNED, 321,     -1, 0);
            CHECK_REG(key, 0, 3, N, TYPE_DOUBLE,   321.12,  -1, 0);
            CHECK_REG(key, 0, 4, N, TYPE_STRING,   "abc",   -1, 0);
                                                      
            CHECK_REG(key, 1, 0, N, TYPE_SIGNED,   456,     -1, 0);
            CHECK_REG(key, 1, 1, N, TYPE_SIGNED,   123,     -1, 0);
            CHECK_REG(key, 1, 2, N, TYPE_UNSIGNED, 321,     -1, 0);
            CHECK_REG(key, 1, 3, N, TYPE_DOUBLE,   321.12,  -1, 0);
            CHECK_REG(key, 1, 6, N, TYPE_DOUBLE,   (double)1 / 456,     -1, 0);
            CHECK_REG(key, 1, 7, N, TYPE_SIGNED,   -123,     -1, 0);
            CHECK_REG(key, 1, 8, N, TYPE_UNSIGNED, (uint64_t)-322,     -1, 0);
            CHECK_REG(key, 1, 9, N, TYPE_DOUBLE,   -321.12,  -1, 0);

            CHECK_REG(key, 2, 5, N, TYPE_STRING,   "abc",     -1, 1);

            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}



