#pragma once

#include <stdlib.h>
#include <string.h>

#include "reg.h"
#include "ivt.h"
#include "func.h"
#include "error.h"
#include "scope.h"
#include "context.h"



struct regvm
{
    regs        reg;
    scope       globals;
    context*    ctx     = NULL;
    error       err;
    ivt         idt;
    func        funcs;

    regvm();
    ~regvm();

    bool call(void* arg);
    bool ret(void);
};


