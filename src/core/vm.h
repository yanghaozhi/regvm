#pragma once

#include <stdlib.h>
#include <string.h>

#include "reg.h"
#include "ivt.h"
#include "func.h"
#include "error.h"
//#include "scope.h"
#include "frame.h"

//#include "structs.h"
#include "mlib.h"

#include <map>


typedef int (*vm_op_t)(regvm* vm, int code, int reg, int ex, int offset, int64_t extra);

struct regvm
{
    bool            exit        = false;
    bool            fatal       = false;
    int64_t         exit_code   = 0;
    core::frame*    call_stack  = NULL;
    core::reg       reg;
    //scope           globals;
    core::error     err;
    core::ivt       idt;

    std::map<int32_t, core::func>   funcs;
    std::map<int64_t, const char*>  strs;

    vm_op_t         ops[16];

    regvm();
    virtual ~regvm();

    bool run(const code_t* start, int count);
    bool call(int64_t id, int code, int reg, int ex, int offset);
    bool call(core::reg::v& addr, int code, int reg, int ex, int offset);
};

