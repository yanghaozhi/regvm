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
# $2.from != NULL, $2.ref == 2
TRAP    2   2

# abc.reg == 2, abc.ref == 2
SETS    2   2   456
# $2.from != NULL, $2.ref == 2, $2.value == 456
# $abc.reg == 2, $abc.ref == 2, $abc.value == 123
TRAP    2   3

STORE   2   2
# $2.from != NULL, $2.ref == 2, $2.value == 456
# $abc.reg == 2, $abc.ref == 2, $abc.value == 456
TRAP    2   4

SETC    5   4   def
# def = $2
STORE   2   5
# $2.from != NULL, $2.ref = 2
# $abc.reg == -1, $abc.ref = 1
# $def.reg == 2, $def.ref = 2
TRAP    3   5

EXIT    0   0
)";

TEST(code, store)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 1, 1, N, TYPE_SIGNED,   123,     -1, 0);
            CHECK_REG(key, 1, 2, N, TYPE_UNSIGNED, 321,     -1, 0);
            CHECK_REG(key, 1, 3, N, TYPE_DOUBLE,   321.123, -1, 0);
            CHECK_REG(key, 1, 4, N, TYPE_STRING,   "abc",   -1, 0);
                                                      
            CHECK_REG(key, 2, 2, Y, TYPE_UNSIGNED, 321,     2, 0);
                                                   
            CHECK_REG(key, 3, 2, Y, TYPE_UNSIGNED, 456,     2, 0);

            CHECK_REG(key, 4, 2, Y, TYPE_UNSIGNED, 456,     2, 0);

            CHECK_REG(key, 5, 2, Y, TYPE_UNSIGNED, 456,     2, 0);

            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_VAR(key, 2, "abc", 0, 0, 2,  TYPE_UNSIGNED, 321, 2, 0);

            CHECK_VAR(key, 3, "abc", 0, 0, 2,  TYPE_UNSIGNED, 321, 2, 0);

            CHECK_VAR(key, 4, "abc", 0, 0, 2,  TYPE_UNSIGNED, 456, 2, 0);

            CHECK_VAR(key, 5, "abc", 0, 0, -1, TYPE_UNSIGNED, 456, 1, 0);
            CHECK_VAR(key, 5, "def", 0, 0, 2,  TYPE_UNSIGNED, 456, 2, 0);
            return match;
            //printf("%d - %d\t%d\t%d\t%s\t%d(%s)\t%d\t%d\t%p\n", key, info->type, info->reg, info->ref, info->var_name, info->func_id, info->func_name, info->call_id, info->scope_id, info->raw);
            //printf("%d\n", (strcmp("abc", info->var_name) == 0));
        });
    ASSERT_EQ(0, t.go(txt));
}

