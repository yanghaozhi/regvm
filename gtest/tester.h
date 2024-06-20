#pragma once

#include <gtest/gtest.h>

#include <mem_run.h>
#include <debugger.h>

class tester : public vasm::debugger
{
public:
    int64_t go(char* txt);

protected:
    union 
    {
        code_t      code;
        uint16_t    key;
    }               cur;
    int             offset;
    virtual void trap(regvm* vm, code_t code, int offset);

    virtual void check_reg(const regvm_reg_info* info);
    virtual void check_var(const regvm_var_info* info);

private:
    static void check_reg(void* arg, const regvm_reg_info* info);
    static void check_var(void* arg, const regvm_var_info* info);
};
