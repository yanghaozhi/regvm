#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SETS    1   1   123
SETS    2   2   321
SETD    3   3   321.123
SETC    4   4   abc
# watch ：1/2/3/4
TRAP    4   1
STR     0   0   4
TRAP    1   2
CMD     0   0   1   0
EXIT    0   0
)";

TEST(cmd, str)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 1, 1, N, TYPE_SIGNED,   123,     -1);
            CHECK_REG(key, 1, 2, N, TYPE_UNSIGNED, 321,     -1);
            CHECK_REG(key, 1, 3, N, TYPE_DOUBLE,   321.123, -1);
            CHECK_REG(key, 1, 4, N, TYPE_STRING,   "abc",   -1);

            CHECK_REG(key, 2, 0, N, TYPE_SIGNED,   3,   -1);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            return match;
            //printf("%d - %d\t%d\t%d\t%s\t%d(%s)\t%d\t%d\t%p\n", key, info->type, info->reg, info->ref, info->var_name, info->func_id, info->func_name, info->call_id, info->scope_id, info->raw);
            //printf("%d\n", (strcmp("abc", info->var_name) == 0));
        });
    ASSERT_EQ(0, t.go(txt));
}

