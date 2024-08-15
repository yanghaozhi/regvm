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


typedef int (*vm_op_t)(regvm* vm, code_t inst, int offset, const void* extra);

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

    vm_op_t         ops[256 - CODE_TRAP];

    regvm();
    virtual ~regvm();

    bool run(const code_t* start, int count);
    bool call(int64_t id, code_t code, int offset);
    bool call(core::reg::v& addr, code_t code, int offset);

#define CRTP_FUNC(name, ret, val, argc, ...)                                \
    virtual ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__))    {return val;};

    CRTP_FUNC(vm_init,  bool, true, 0);
    CRTP_FUNC(vm_exit,  bool, true, 0);
    CRTP_FUNC(vm_call,  bool, true, 3, code_t, int, int64_t);
    CRTP_FUNC(vm_var,   core::var*, NULL, 1, int);
    CRTP_FUNC(vm_var,   core::var*, NULL, 2, int, uint64_t);

#undef CRTP_FUNC

#ifdef DEBUG
    const char* code_names[256];
#define CODE_NAME(x)    vm->code_names[x]
#else
#define CODE_NAME(x)    ""
#endif
};

