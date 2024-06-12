#pragma once

#include <stdlib.h>
#include <string.h>

#include "reg.h"
#include "ivt.h"
#include "func.h"
#include "error.h"
#include "scope.h"
#include "context.h"

#include <map>


struct regvm
{
    regs        reg;
    scope       globals;
    context*    ctx     = NULL;
    error       err;
    ivt         idt;

    std::map<uint64_t, func>   funcs;

    regvm();
    ~regvm();

    bool run(const code_t* start, int count);
    bool call(uint64_t id, code_t code, int offset);
    bool ret(void);
};


