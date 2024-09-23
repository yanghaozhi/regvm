#include "tester.h"

#include <irq.h>

#include <gtest/gtest.h>


int64_t test_base::go(char* txt)
{
    return go(txt, true);
}

int64_t test_base::go(char* txt, bool expect)
{
    vasm::mem_run mem;

    if (mem.go(txt) == false)
    {
        LOGE("parse file : %s ERROR !!!", txt);
        return -1;
    }

    char* buf = NULL;
    size_t size = 0;
    FILE* fp = open_memstream(&buf, &size);

    if (mem.finish(fp, &inst::print_bin) == false)
    {
        LOGE("finish ERROR : %p", fp);
        return 1;
    }

    fclose(fp);

    //if LOG_IS_ENBALE(DEBUG)
    {
        const int line = 16;
        int j = 0;
        const unsigned char* p = (const unsigned char*)buf;
        for (int i = 0; i < (int)size; i++)
        {
            if (j++ >= line)
            {
                printf("\n");
                j = 1;
            }
            printf("%02X ", p[i]);
        }
        printf("\n\n");
    }

    extern int mem_init(void);
    auto vm = regvm_init(1, mem_init);

    regvm_irq_set(vm, IRQ_TRAP, debug_trap, this);

    int64_t exit = 0;
    bool r = regvm_exec(vm, (code_t*)buf, size >> 2, &exit);

    regvm_exit(vm);

    LOGI("run : %d\n", r);

    free(buf);

    return exit;
}

void test_base::trap(regvm* vm, code_t code, int offset)
{
    //key = code.reg;
    //key <<= 4;
    //key += code.ex;
    key = code.a;
    this->offset = offset;
    trap_arg arg;
    arg.self = this;
    arg.matches = 0;
    regvm_debug_reg_callback(vm, check_reg, &arg);
    EXPECT_EQ((int)code.b, arg.matches) << " at TRAP reg : " << key;

    arg.matches = 0;
    regvm_debug_var_callback(vm, check_var, &arg);
    EXPECT_EQ((int)code.c, arg.matches) << " at TRAP var : " << key;
}

int64_t test_base::debug_trap(regvm* vm, void* arg, code_t code, int offset, void* extra)
{
    test_base* self = (test_base*)arg;
    self->trap(vm, code, offset);
    return 1;
}

int test_base::check_reg(const regvm_reg_info* info)
{
    return 0;
}

int test_base::check_var(const regvm_var_info* info)
{
    return 0;
}

void test_base::check_reg(void* arg, const regvm_reg_info* info)
{
    auto i = (intptr_t)info;
    auto p = (trap_arg*)arg;
    if ((i != 0) && (i != -1))
    {
        p->matches += p->self->check_reg(info);
    }
}

void test_base::check_var(void* arg, const regvm_var_info* info)
{
    auto i = (intptr_t)info;
    auto p = (trap_arg*)arg;
    if ((i != 0) && (i != -1))
    {
        p->matches += p->self->check_var(info);
    }
}

