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


typedef int (*vm_op_t)(regvm* vm, int code, int reg, int ex, int offset);

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

#define CRTP_FUNC(name, ret, argc, ...)                                             \
    virtual ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__))    {return 0;};

    CRTP_FUNC(vm_init,  bool, 0);
    CRTP_FUNC(vm_exit,  bool, 0);
    CRTP_FUNC(vm_var,   core::var*, 1, int);
    CRTP_FUNC(vm_var,   core::var*, 2, int, const char*);
    CRTP_FUNC(vm_call,  bool, 5, int, int, int, int, int64_t);

#undef CRTP_FUNC
};

