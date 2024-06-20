#include "tester.h"

#include <gtest/gtest.h>

int64_t tester::go(char* txt)
{
    vasm::mem_2_run vm(NULL);

    vm.set_dbg(this);

    EXPECT_TRUE(vm.open(txt, strlen(txt)));

    vasm::mem_2_run::pass1 s1(vm);
    EXPECT_TRUE(s1.scan());

    vasm::mem_2_run::pass2 s2(vm);
    EXPECT_TRUE(s2.scan());

    EXPECT_TRUE(vm.finish());

    return exit;
}

void tester::trap(regvm* vm, code_t code, int offset)
{
    cur.code = code;
    this->offset = offset;
    regvm_debug_reg_callback(vm, check_reg, this);
    regvm_debug_var_callback(vm, check_var, this);
}

void tester::check_reg(const regvm_reg_info* info)
{
}

void tester::check_var(const regvm_var_info* info)
{
}

void tester::check_reg(void* arg, const regvm_reg_info* info)
{
    auto i = (intptr_t)info;
    if ((i != 0) && (i != -1))
    {
        ((tester*)arg)->check_reg(info);
    }
}

void tester::check_var(void* arg, const regvm_var_info* info)
{
    auto i = (intptr_t)info;
    if ((i != 0) && (i != -1))
    {
        ((tester*)arg)->check_var(info);
    }
}

