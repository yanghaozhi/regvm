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
# abc = $2
STORE   2   4
# $2.from != NULL, $2.ref = 2
TRAP    2   2
# abc.reg == 2, abc.ref = 2
SETS    2   1   456
# $2.from == NULL, $2.ref = -1
# abc.reg == -1, abc.ref = 1
TRAP    2   3
EXIT    0   0
)";

TEST(code, set)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 1, 1, EQ, TYPE_SIGNED,   123,     -1);
            CHECK_REG(key, 1, 2, EQ, TYPE_UNSIGNED, 321,     -1);
            CHECK_REG(key, 1, 3, EQ, TYPE_DOUBLE,   321.123, -1);
            CHECK_REG(key, 1, 4, EQ, TYPE_STRING,   "abc",   -1);
                                                      
            CHECK_REG(key, 2, 2, NE, TYPE_UNSIGNED, 321,     2);
                                                   
            CHECK_REG(key, 3, 2, EQ, TYPE_SIGNED,   456,     -1);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_VAR(key, 2, "abc", 0, 0, 2,  TYPE_UNSIGNED, 321, 2);
            CHECK_VAR(key, 3, "abc", 0, 0, -1, TYPE_UNSIGNED, 321, 1);
            return match;
            //printf("%d - %d\t%d\t%d\t%s\t%d(%s)\t%d\t%d\t%p\n", key, info->type, info->reg, info->ref, info->var_name, info->func_id, info->func_name, info->call_id, info->scope_id, info->raw);
            //printf("%d\n", (strcmp("abc", info->var_name) == 0));
        });
    ASSERT_EQ(0, t.go(txt));
}

