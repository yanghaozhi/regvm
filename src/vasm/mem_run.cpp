#include "mem_run.h"

#include <regvm.h>
#include <debug.h>
#include <irq.h>

#include "../ext/regext.h"

using namespace vasm;

mem_2_run::mem_2_run() : buf(NULL)
{
}

mem_2_run::~mem_2_run()
{
    free(buf);
}

bool mem_2_run::finish()
{
    const int line = 16;
    int j = 0;
    const unsigned char* p = (const unsigned char*)buf;
    INFO("total {} bytes of codes", code_bytes);
    for (int i = 0; i < code_bytes; i++)
    {
        if (j++ >= line)
        {
            printf("\n");
            j = 1;
        }
        printf("%02X ", p[i]);
    }
    printf("\n\n");

    auto vm = regvm_init(&var_ext);

    regvm_irq_set(vm, IRQ_TRAP, debug_trap, NULL);

    int64_t exit = 0;
    bool r = regvm_exec(vm, codes, code_bytes >> 1, &exit);
    INFO("run result : {} and exit code : {}", r, exit);

    regvm_exit(vm);
    return r;
}

mem_2_run::pass1::pass1(mem_2_run& o) : strs::pass1(o), data(o)
{
}

int mem_2_run::pass1::write_code(const code_t* code, int bytes)
{
    data.code_bytes += bytes;
    return bytes;
}

mem_2_run::pass2::pass2(mem_2_run& o) : strs::pass2(o), data(o)
{
    data.buf = malloc(data.code_bytes);
    cur = data.codes;
}

int mem_2_run::pass2::write_code(const code_t* code, int bytes)
{
    memcpy(cur, code, bytes);
    cur += (bytes >> 1);
    return bytes;
}

//bool mem_2_run::debug_reg(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
//{
//    //regvm_reg_info info;
//    //memset(&info, 0, sizeof(info));
//    //cb(arg, NULL);
//    //error::reg_info(vm->reg, [cb, arg](const regvm_reg_info* info)
//    //        {
//    //            cb(arg, info);
//    //        }, &info);
//    //cb(arg, (regvm_reg_info*)(intptr_t)-1);
//    return true;
//}
//
//bool mem_2_run::debug_var(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
//{
//    //regvm_var_info info;
//    //memset(&info, 0, sizeof(info));
//    //cb(arg, NULL);
//    //error::ctx_vars(*vm->ctx, [cb, arg](const regvm_var_info* info)
//    //        {
//    //            cb(arg, info);
//    //        }, &info);
//    //cb(arg, (regvm_var_info*)(intptr_t)-1);
//    return true;
//}
struct dump_arg
{
    int     reg;
};

int64_t mem_2_run::debug_trap(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
{
    printf("%d :\n", offset);
    struct dump_arg a;
    switch (code.ex)
    {
    case 0: //all regs
        a.reg = -1;
        regvm_debug_reg_callback(vm, dump_reg_info, &a);
        break;
    case 1: //1 arg
        a.reg = code.reg;
        regvm_debug_reg_callback(vm, dump_reg_info, &a);
        break;
    //case 2:
    //    regvm_debug_var_callback(vm, dump_var_info, &arg);
    //    break;
    default:
        break;
    }
    return true;
}

void mem_2_run::dump_reg_info(void* arg, const regvm_reg_info* info)
{
    auto p = (struct dump_arg*)arg;

    switch ((intptr_t)info)
    {
    case 0:
        printf("\e[33m id\ttype\tref\tvar\tvalue \e[0m\n");
        break;
    case -1:
        break;
    default:
        if ((p->reg < 0) || (p->reg == info->id))
        {
            printf(" %d\t%d\t%d\t%p\t", info->id, info->type, info->ref, info->from);
            regvm_debug_uvalue_print(info->type, info->value);
            printf("\n");
        }
        break;
    }
}

