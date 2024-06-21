#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(#CODE_SET
# $1 = 123
SETS    1   1   123
SETS    2   2   321
SETD    3   3   321.123
SETC    4   4   abc
TRAP    0   1
# abc = $2
STORE   2   4
TRAP    0   2
TRAP    0   3
SETS    2   1   456
TRAP    0   4
EXIT    0   0
)";

TEST(vcore, set_value)
{
    tester t([](auto key, auto offset, auto info)
        {
            bool match = false;
            CHECK_REG(key, 1, 1, EQ, I, 123,    1, -1);
            CHECK_REG(key, 1, 2, EQ, I, 321,    2, -1);
            CHECK_REG(key, 1, 3, EQ, F, 321.123,3, -1);
            CHECK_REG(key, 1, 4, EQ, S, "abc",  4, -1);
                                          
            CHECK_REG(key, 2, 2, NE, I, 321,    2,  2);

            CHECK_REG(key, 4, 2, EQ, I, 456,    1,  -1);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            bool match = false;
            CHECK_VAR(key, 3, "abc", 0, 0, 2, I, 321,    2, 2);
            CHECK_VAR(key, 4, "abc", 0, 0, -1, I, 321,    2, 1);
            return match;
            //printf("%d - %d\t%d\t%d\t%s\t%d(%s)\t%d\t%d\t%p\n", key, info->type, info->reg, info->ref, info->var_name, info->func_id, info->func_name, info->call_id, info->scope_id, info->raw);
            //printf("%d\n", (strcmp("abc", info->var_name) == 0));
        });
    ASSERT_EQ(0, t.go(txt));
}
