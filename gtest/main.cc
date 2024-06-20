#include <stdio.h>
#include <gtest/gtest.h>

#include <log.h>

int main(int argc, char** argv)
{
    logging::set_level(logging::INFO);
    //logging::set_level(0);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
