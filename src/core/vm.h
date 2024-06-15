#pragma once

#include <stdlib.h>
#include <string.h>

#include "reg.h"
#include "ivt.h"
#include "func.h"
#include "error.h"
//#include "scope.h"
#include "frame.h"

#include "structs.h"

#include <map>



typedef bool (*vm_ext_handler_t)(struct regvm* vm, const code_t code, int offset, int64_t value);

struct regvm;

struct regvm_ex
{
    bool (*init)(regvm* vm);
    bool (*exit)(regvm* vm);

    vm_ext_handler_t    vm_set;
    vm_ext_handler_t    vm_store;
    vm_ext_handler_t    vm_load;
    vm_ext_handler_t    vm_block;       //value 0 enter, 1 leave
    vm_ext_handler_t    vm_call;        //value is function id, > 0 call, < 0 return
};

struct regvm
{
    bool        exit        = false;
    bool        fatal       = false;
    int64_t     exit_code   = 0;
    frame*      call_stack  = NULL;
    regs        reg;
    //scope       globals;
    error       err;
    ivt         idt;
    regvm_ex    handlers;
    void*       ext         = NULL; //for ext

    std::map<int64_t, func>   funcs;

    regvm(struct regvm_ex* ext);
    ~regvm();

    bool run(const code_t* start, int count);
    bool call(int64_t id, const code_t code, int offset);
    bool ret(void);
};


