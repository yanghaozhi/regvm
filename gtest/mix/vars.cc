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
TRAP    0   0
# abc = $0
STORE   0   3
TRAP    0   1
BLOCK   0   0   
# $4 = "def"
SETC    4   4   def
# def = $2
STORE   2   4   
TRAP    0   2
# $4 = "qwer"
SETC    5   4   qwer
# $6 = "abc"
SETC    6   4   abc
# abc = $1
STORE   1   3   
TRAP    0   3
TRAP    0   4
BLOCK   0   1
TRAP    0   5
TRAP    0   6
# $0 += 10
INC     0   10
#TRAP    0   1
# $0 -= 5
DEC     0   5
#TRAP    0   1
#TRAP    1   1
# $1 += $0
ADD     1   0
#TRAP    1   1
# $1 -= $2
SUB     1   2
#TRAP    1   1
# $1 = 1 / $1
CHG     1   2
#TRAP    1   1
#TRAP    0   1
)";

TEST(mix, vars)
{
    tester t([](auto key, auto offset, auto info)
        {
            bool match = false;
            CHECK_REG(key, 0, 0, EQ, I, 456,    TYPE_UNSIGNED, -1);
            CHECK_REG(key, 0, 1, EQ, I, 123,    TYPE_SIGNED, -1);
            CHECK_REG(key, 0, 2, EQ, I, 321,    TYPE_UNSIGNED, -1);
            CHECK_REG(key, 0, 3, EQ, S, "abc",  TYPE_STRING, -1);

            CHECK_REG(key, 1, 0, NE, I, 456,    TYPE_UNSIGNED,  2);

            CHECK_REG(key, 2, 2, NE, I, 321,    TYPE_UNSIGNED,  2);

            CHECK_REG(key, 3, 1, NE, I, 123,    TYPE_SIGNED,  2);

            CHECK_REG(key, 5, 1, EQ, I, 123,    TYPE_SIGNED,  -1);
            return match;
        },
        [](auto key, auto offset, auto info)
        {
            bool match = false;
            CHECK_VAR(key, 1, "abc", 0, 0, 0,  I, 456, TYPE_UNSIGNED, 2);
            CHECK_VAR(key, 2, "def", 0, 1, 2,  I, 321, TYPE_UNSIGNED, 2);

            //two var named abc in diff scope here
            CHECK_VAR(key, 4, "abc", 0, 0, 0,  I, 456, TYPE_UNSIGNED, 2);
            CHECK_VAR(key, 4, "abc", 0, 1, 1,  I, 123, TYPE_SIGNED, 2);

            //one abc now
            CHECK_VAR(key, 6, "abc", 0, 0, 0,  I, 456, TYPE_UNSIGNED, 2);
            return match;
        });
    ASSERT_EQ(461, t.go(txt));
}

