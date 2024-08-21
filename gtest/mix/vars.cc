#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SET     1   1   123
# $0 = 456
SET     0   2   456
# $2 = 321
SET     2   2   321
# $3 = "abc"
SET     3   7   1
TRAP    0   4   0
# abc = $0
STORE   0   3   2
TRAP    1   1   1
BLOCK   0   0   0
# $4 = "def"
SET     4   7   2
# def = $2
STORE   2   4   2
TRAP    2   1   1
# $4 = "qwer"
SET     5   4   qwer
# $6 = "abc"
SET     6   4   abc
# abc = $1
#STORE   1   3   2
#TRAP    3   1   2
BLOCK   0   1   0
#TRAP    4   1   1
)";

TEST(mix, vars)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 0, 0, N, TYPE_UNSIGNED, 456,    -1, 0);
            CHECK_REG(key, 0, 1, N, TYPE_SIGNED,   123,    -1, 0);
            CHECK_REG(key, 0, 2, N, TYPE_UNSIGNED, 321,    -1, 0);
            CHECK_REG(key, 0, 3, N, TYPE_ADDR,      1,  -1, 0);
                                                  
            CHECK_REG(key, 1, 0, Y, TYPE_UNSIGNED, 456,    2, 0);
                                                  
            CHECK_REG(key, 2, 2, Y, TYPE_UNSIGNED, 321,    2, 0);
                                                  
            CHECK_REG(key, 3, 1, Y, TYPE_SIGNED,   123,    2, 0);
                                                  
            CHECK_REG(key, 4, 1, Y, TYPE_SIGNED,   123,    1, 0);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_VAR(key, 1, 1, 0, 0, 0,  TYPE_UNSIGNED, 456, 2, 0);
            CHECK_VAR(key, 2, 2, 0, 0, 2,  TYPE_UNSIGNED, 321, 2, 0);
                                                              
            ////two var named abc in diff scope here
            //CHECK_VAR(key, 3, 1, 0, 0, 0,  TYPE_UNSIGNED, 456, 2, 0);
            //CHECK_VAR(key, 3, 1, 0, 1, 1,  TYPE_SIGNED,   123, 2, 0);
                                                              
            //one abc now                                     
            CHECK_VAR(key, 4, 1, 0, 0, 0,  TYPE_UNSIGNED, 456, 2, 0);
            return match;
        });
    ASSERT_EQ(456, t.go(txt));
}

