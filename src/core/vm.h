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
#include "ext.h"
#include "mlib.h"

#include <map>



struct regvm
{
    bool            exit        = false;
    bool            fatal       = false;
    int64_t         exit_code   = 0;
    core::frame*    call_stack  = NULL;
    const char*     str_tab     = NULL;

    core::reg       reg;
    //scope           globals;
    core::error     err;
    core::ivt       idt;

    std::map<int32_t, core::func>   funcs;


    regvm();
    virtual ~regvm();

    bool run(const code_t* start, int count);
    bool call(int32_t id, code_t code, int offset);

#ifdef DEBUG
    const char* code_names[256];
#define CODE_NAME(x)    vm->code_names[x]
#else
#define CODE_NAME(x)    ""
#endif

    struct ext
    {
    };
    ext*            exts[0];
};

