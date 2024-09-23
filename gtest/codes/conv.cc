#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SET     1   1   123
SET     2   2   321
SET     3   3   321.12
SET     4   7   100
# watch ：1/2/3/4
TRAP    0   4   0

MOVE    5   3   1
MOVE    6   1   3
MOVE    7   2   1
TRAP    1   6   0

STORE   1   4   1
# $1.from != NULL, $1.ref == 2
# ${100}.reg == 1, ${100}.ref == 2
TRAP    2   1   1
MOVE    1   1   1
# $1.from == NULL, $1.ref == -1, $1.type == 1
# ${100}.reg == 1, ${100}.ref == 2, ${100}.type == 3
TRAP    3   1   1
EXIT    0   0   0
)";

TEST(code, conv)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;

            CHECK_REG(key, 0, 1, N, TYPE_SIGNED,    123,    -1, 0);
            CHECK_REG(key, 0, 2, N, TYPE_UNSIGNED,  321,    -1, 0);
            CHECK_REG(key, 0, 3, N, TYPE_DOUBLE,    321.12, -1, 0);
            CHECK_REG(key, 0, 4, N, TYPE_ADDR,      100,  -1, 0);
                                                       
            CHECK_REG(key, 1, 1, N, TYPE_SIGNED,    123,    -1, 0);
            CHECK_REG(key, 1, 2, N, TYPE_UNSIGNED,  321,    -1, 0);
            CHECK_REG(key, 1, 3, N, TYPE_DOUBLE,    321.12, -1, 0);
            CHECK_REG(key, 1, 5, N, TYPE_SIGNED,    321,    -1, 0);
            CHECK_REG(key, 1, 6, N, TYPE_DOUBLE,    123.0,  -1, 0);
            CHECK_REG(key, 1, 7, N, TYPE_SIGNED,    321,    -1, 0);
                                                    
            CHECK_REG(key, 2, 1, Y, TYPE_SIGNED,    123,  2, 0);
                                                    
            CHECK_REG(key, 3, 1, N, TYPE_SIGNED,    123,  -1, 0);

            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;

            CHECK_VAR(key, 2, 100, 0, 1,  TYPE_SIGNED, 123, 2, 0);
                                                           
            CHECK_VAR(key, 3, 100, 0, -1, TYPE_SIGNED, 123, 1, 0);
            return match;
        });
    ASSERT_EQ(0, t.go(txt));
}


