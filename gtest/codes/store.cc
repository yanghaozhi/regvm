#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SET     1   1   123
SET     2   2   321
SET     3   3   321.123
SET     4   7   100
# watch ：1/2/3/4
TRAP    1   4   0

# ${100} = $2
STORE   2   4   1
# $2.from != NULL, $2.ref == 2
TRAP    2   1   1

# ${100}.reg == 2, ${100}.ref == 2
SET     2   2   456
# $2.from != NULL, $2.ref == 2, $2.value == 456
# $${100}.reg == 2, $${100}.ref == 2, $${100}.value == 123
TRAP    3   1   1

STORE   2   2   0
# $2.from != NULL, $2.ref == 2, $2.value == 456
# $${100}.reg == 2, $${100}.ref == 2, $${100}.value == 456
TRAP    4   1   1

SET     5   7   101
# ${101} = $2
STORE   2   5   1
# $2.from != NULL, $2.ref = 2
# $${100}.reg == -1, $${100}.ref = 1
# $${101}.reg == 2, $${101}.ref = 2
TRAP    5   1   2

EXIT    0   0   0
)";

TEST(code, store)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 1, 1, N, TYPE_SIGNED,    123,     -1, 0);
            CHECK_REG(key, 1, 2, N, TYPE_UNSIGNED,  321,     -1, 0);
            CHECK_REG(key, 1, 3, N, TYPE_DOUBLE,    321.123, -1, 0);
            CHECK_REG(key, 1, 4, N, TYPE_ADDR,      100,   -1, 0);
                                                       
            CHECK_REG(key, 2, 2, Y, TYPE_UNSIGNED,  321,     2, 0);
                                                    
            CHECK_REG(key, 3, 2, Y, TYPE_UNSIGNED,  456,     2, 0);

            CHECK_REG(key, 4, 2, Y, TYPE_UNSIGNED,  456,     2, 0);

            CHECK_REG(key, 5, 2, Y, TYPE_UNSIGNED,  456,     2, 0);

            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_VAR(key, 2, 100, 0, 2,  TYPE_UNSIGNED, 321, 2, 0);

            CHECK_VAR(key, 3, 100, 0, 2,  TYPE_UNSIGNED, 321, 2, 0);

            CHECK_VAR(key, 4, 100, 0, 2,  TYPE_UNSIGNED, 456, 2, 0);

            CHECK_VAR(key, 5, 100, 0, -1, TYPE_UNSIGNED, 456, 1, 0);
            CHECK_VAR(key, 5, 101, 0, 2,  TYPE_UNSIGNED, 456, 2, 0);
            return match;
            //printf("%d - %d\t%d\t%d\t%s\t%d(%s)\t%d\t%d\t%p\n", key, info->type, info->reg, info->ref, info->var_name, info->func_id, info->func_name, info->call_id, info->scope_id, info->raw);
            //printf("%d\n", (strcmp("abc", info->var_name) == 0));
        });
    ASSERT_EQ(0, t.go(txt));
}

