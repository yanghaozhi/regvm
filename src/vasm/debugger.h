#pragma once

#include <regvm.h>
#include <debug.h>

namespace vasm
{

class debugger
{
public:
    enum
    {
        REG = 1,
        VAR = 2,
    };

    bool start(regvm* vm, uint64_t mode);

    int64_t         exit    = -1;

protected:
    uint64_t        mode    = 0;
    code_t          cur;

    virtual void reg_info(const regvm_reg_info* info);
    virtual void var_info(const regvm_var_info* info);

    virtual void trap(regvm* vm, code_t code, int offset);

    static void dump_reg_info(void* arg, const regvm_reg_info* info);
    static void dump_var_info(void* arg, const regvm_var_info* info);
    static int64_t debug_trap(regvm* vm, void*, code_t code, int offset, void* extra);
};

};

