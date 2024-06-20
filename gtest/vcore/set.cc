#include <gtest/gtest.h>

#include "../tester.h"

char asms[] = R"(
# $1 = 12
SETS    1   1   12
TRAP    1   1
# $0 = 0
CLEAR   0   1
TRAP    0   1
# $0 += $1
#LABEL: aaa
ADD     0   1
TRAP    0   1
# $1 -= 1
DEC     1   1
TRAP    1   1
# $2 = -6
#SETS    2   8   -7
SETC    2   9   #LABEL:  aaa
TRAP    15   1
# if ($1 > 0) jump $2
JG      1   2
TRAP    0   1
# $0 += 10
INC     0   10
TRAP    0   1
)";

TEST(core, SET)
{
    tester t;
    //ASSERT_TRUE(t.test(asms));

    //ASSERT_TRUE(t.open(asms, strlen(asms)));

    //vasm::mem_2_run::pass1 s1(t);
    //ASSERT_TRUE(s1.scan());

    //vasm::mem_2_run::pass2 s2(t);
    //ASSERT_TRUE(s2.scan());

    //ASSERT_TRUE(t.finish());
}
