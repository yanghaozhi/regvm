#include <gtest/gtest.h>

#include "../tester.h"

static char txt[] = R"(
# $1 = 123
SETS    1   1   123
# $0 = 456
SETL    0   2   456
# $2 = 321
SETI    2   2   321
# $3 = "abc"
SETC    3   4   abc
TRAP    4   0
# abc = $0
STORE   0   3
TRAP    2   1
BLOCK   0   0   
# $4 = "def"
SETC    4   4   def
# def = $2
STORE   2   4   
TRAP    2   2
# $4 = "qwer"
SETC    5   4   qwer
# $6 = "abc"
SETC    6   4   abc
# abc = $1
STORE   1   3   
TRAP    3   3
BLOCK   0   1
TRAP    2   4
)";

TEST(mix, vars)
{
    tester t([](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_REG(key, 0, 0, N, TYPE_UNSIGNED, 456,    -1, 0);
            CHECK_REG(key, 0, 1, N, TYPE_SIGNED,   123,    -1, 0);
            CHECK_REG(key, 0, 2, N, TYPE_UNSIGNED, 321,    -1, 0);
            CHECK_REG(key, 0, 3, N, TYPE_STRING,   "abc",  -1, 0);
                                                  
            CHECK_REG(key, 1, 0, Y, TYPE_UNSIGNED, 456,    2, 0);
                                                  
            CHECK_REG(key, 2, 2, Y, TYPE_UNSIGNED, 321,    2, 0);
                                                  
            CHECK_REG(key, 3, 1, Y, TYPE_SIGNED,   123,    2, 0);
                                                  
            CHECK_REG(key, 4, 1, Y, TYPE_SIGNED,   123,    1, 0);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            int match = 0;
            CHECK_VAR(key, 1, "abc", 0, 0, 0,  TYPE_UNSIGNED, 456, 2, 0);
            CHECK_VAR(key, 2, "def", 0, 1, 2,  TYPE_UNSIGNED, 321, 2, 0);
                                                              
            //two var named abc in diff scope here
            CHECK_VAR(key, 3, "abc", 0, 0, 0,  TYPE_UNSIGNED, 456, 2, 0);
            CHECK_VAR(key, 3, "abc", 0, 1, 1,  TYPE_SIGNED,   123, 2, 0);
                                                              
            //one abc now                                     
            CHECK_VAR(key, 4, "abc", 0, 0, 0,  TYPE_UNSIGNED, 456, 2, 0);
            return match;
        });
    ASSERT_EQ(456, t.go(txt));
}

