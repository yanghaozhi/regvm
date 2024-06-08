#pragma once

#include <stdlib.h>
#include <string.h>

#include "reg.h"
#include "ivt.h"
#include "error.h"
#include "scope.h"
#include "context.h"



struct regvm
{
    regs        reg;
    scope       globals;
    context*    ctx;
    error       err;
    ivt         idt;

    regvm();
    ~regvm();

    bool call(void);
    bool ret(void);
};


