#include "tester.h"

#include <gtest/gtest.h>

tester::tester() : mem_2_run(NULL)
{
}

bool tester::test(char* asms)
{
    ASSERT_TRUE(mem_2_run::open(asms, strlen(asms)));

    //vasm::mem_2_run::pass1 s1(*this);
    //ASSERT_TRUE(s1.scan());

    //vasm::mem_2_run::pass2 s2(*this);
    //ASSERT_TRUE(s2.scan());

    //ASSERT_TRUE(finish());

    return true;
}
