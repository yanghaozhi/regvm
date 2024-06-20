#pragma once

#include <mem_run.h>

class tester : public vasm::mem_2_run
{
public:
    tester();

    bool test(char* asms);
};
