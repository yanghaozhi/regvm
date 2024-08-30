#pragma once

#include <stdlib.h>
#include <string.h>

#include "mlib.h"
#include "ext_type.h"



struct regvm;

extern "C"
{
    typedef int (*vm_op_t)(regvm* vm, code_t inst, int offset, const void* extra);
    extern vm_op_t      vm_code_ops[256 - CODE_TRAP];

    typedef bool (*vm_ext_t)(regvm* vm, int idx, void* arg);
    extern void vm_add_ext(void* arg, vm_ext_t init, vm_ext_t exit);
};

namespace core
{
    class var;
    class regv;
};

struct regvm_ext_op
{
    core::var*  (*var_create)(regvm* vm, int type, uint64_t id);
    core::var*  (*var_create_from_reg)(regvm* vm, int reg_id);

    bool        (*var_set_val)(core::var* var, const core::regv& r);
    bool        (*var_set_reg)(const core::var* var, const core::regv* r);
    bool        (*var_release)(const core::var* var);
};

extern regvm_ext_op     vm_ext_ops;
