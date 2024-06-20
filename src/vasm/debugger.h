#pragma once

#include <regvm.h>
#include <debug.h>

class debugger
{
public:
    enum
    {
        REG = 1,
        VAR = 2,
    };

    bool start(regvm* vm, uint64_t mode);

private:
    uint64_t        mode    = 0;

    struct reg_arg
    {
        debugger*   self;
        int         reg_id;
    };

    virtual void reg_info(reg_arg* arg, const regvm_reg_info* info);

    struct var_arg
    {
        debugger*   self;
    };
    virtual void var_info(var_arg* arg, const regvm_var_info* info);

    static void dump_reg_info(void* arg, const regvm_reg_info* info);
    static void dump_var_info(void* arg, const regvm_var_info* info);
    static int64_t debug_trap(regvm* vm, void*, code_t code, int offset, void* extra);
};
