#include <stdio.h>
#include <gtest/gtest.h>

#include <log.h>

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
