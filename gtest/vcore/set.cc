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

#define CHECK_REG(ID, VAL, TYPE, REF, FROM)     \
    if (info->id == ID)                         \
    {                                           \
        EXPECT_EQ(VAL, info->value.sint);       \
        EXPECT_EQ(TYPE, info->type);            \
        EXPECT_EQ(REF, info->ref);              \
        EXPECT_EQ(FROM, info->from);            \
    }

class sssbbb : public tester
{
public:
    sssbbb() : tester() {}

    virtual void check_reg(const regvm_reg_info* info)
    {
        //static int v = 0;
        switch (offset)
        {
        case 2:
            CHECK_REG(1, 12, 1, -1, nullptr);
            break;
        case 4:
            CHECK_REG(0, 0, 1, -1, nullptr);
            break;
        case 16:
            CHECK_REG(0, 78, 1, -1, nullptr);
            break;
        case 18:
            CHECK_REG(0, 88, 1, -1, nullptr);
            break;
        default:
            break;
        }
    }

    virtual void check_var(const regvm_var_info* info)
    {
    };
};

TEST(core, SET)
{
    sssbbb sb;

    EXPECT_EQ(88, sb.go(txt));

    //return true;
}

