#include <gtest/gtest.h>

#include "../tester.h"

char txt[] = R"(
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

class sssbbb : public tester
{
public:
    sssbbb() : tester() {}

    virtual void check_reg(const regvm_reg_info* info)
    {
        CHECK_REG(2, 1, 12, 1, -1, nullptr);
        CHECK_REG(4, 0, 0, 1, -1, nullptr);
        CHECK_REG(16, 0, 78, 1, -1, nullptr);
        CHECK_REG(18, 0, 88, 1, -1, nullptr);
    }

    virtual void check_var(const regvm_var_info* info)
    {
    };
};

TEST(core, SET)
{
    sssbbb sb;

    ASSERT_EQ(88, sb.go(txt));

    //return true;
}

