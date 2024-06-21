#include "tester.h"

#include <gtest/gtest.h>

int64_t test_base::go(char* txt)
{
    vasm::mem_2_run vm(NULL);

    vm.set_dbg(this);

    EXPECT_TRUE(vm.open(txt, strlen(txt)));

    vasm::mem_2_run::pass1 s1(vm);
    EXPECT_TRUE(s1.scan());

    vasm::mem_2_run::pass2 s2(vm);
    EXPECT_TRUE(s2.scan());

    EXPECT_TRUE(vm.finish());

    vm.set_dbg(NULL);

    return exit;
}

void test_base::trap(regvm* vm, code_t code, int offset)
{
    key = code.reg;
    key <<= 4;
    key += code.ex;
    this->offset = offset;
    trap_arg arg;
    arg.self = this;
    arg.result = false;
    regvm_debug_reg_callback(vm, check_reg, &arg);
    regvm_debug_var_callback(vm, check_var, &arg);
    EXPECT_TRUE(arg.result);
}

bool test_base::check_reg(const regvm_reg_info* info)
{
    return true;
}

bool test_base::check_var(const regvm_var_info* info)
{
    return true;
}

void test_base::check_reg(void* arg, const regvm_reg_info* info)
{
    auto i = (intptr_t)info;
    auto p = (trap_arg*)arg;
    if ((i != 0) && (i != -1))
    {
        p->result |= p->self->check_reg(info);
    }
}

void test_base::check_var(void* arg, const regvm_var_info* info)
{
    auto i = (intptr_t)info;
    auto p = (trap_arg*)arg;
    if ((i != 0) && (i != -1))
    {
        p->result |= p->self->check_var(info);
    }
}

