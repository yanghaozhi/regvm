#include "debugger.h"

#include <irq.h>

#include <stdio.h>

using namespace vasm;

debugger::~debugger()
{
}

bool debugger::start(regvm* vm, uint64_t m)
{
    regvm_irq_set(vm, IRQ_TRAP, debug_trap, this);
    mode = m;
    return true;
}

struct dump_arg
{
    int     reg;
};

void debugger::reg_info(const regvm_reg_info* info)
{
    switch ((intptr_t)info)
    {
    case 0:
        printf("\e[33m id\ttype\tref\tvar\tvalue \e[0m\n");
        break;
    case -1:
        break;
    default:
        if ((cur.ex == 0) || (cur.reg == info->id))
        {
            printf(" %d\t%d\t%d\t%p\t", info->id, info->type, info->ref, info->from);
            regvm_debug_uvalue_print(info->type, info->value);
            printf("\n");
        }
        break;
    }
}

void debugger::var_info(const regvm_var_info* info)
{
    switch ((intptr_t)info)
    {
    case 0:
        printf("\e[32m type\treg\tref\tname\tfunc\tcall\tscope\tptr\tvalue\e[0m\n");
        break;
    case -1:
        break;
    default:
        printf(" %d\t%d\t%d\t%s\t%d(%s)\t%d\t%d\t%p\t", info->type, info->reg, info->ref, info->var_name, info->func_id, info->func_name, info->call_id, info->scope_id, info->raw);
        regvm_debug_uvalue_print(info->type, info->value);
        printf("\n");
        break;
    }
}

void debugger::trap(regvm* vm, code_t code, int offset)
{
    printf("%d :\n", offset);
    cur = code;
    switch (code.ex)
    {
    case 0: //all regs
        if ((mode & REG) != 0)
        {
            regvm_debug_reg_callback(vm, dump_reg_info, this);
        }
        break;
    case 1: //1 arg
        regvm_debug_reg_callback(vm, dump_reg_info, this);
        break;
    case 2:
        if ((mode & VAR) != 0)
        {
            regvm_debug_var_callback(vm, dump_var_info, this);
        }
        break;
    default:
        break;
    }
}

int64_t debugger::debug_trap(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
{
    auto d = (debugger*)arg;
    d->trap(vm, code, offset);
    return 1;
}

void debugger::dump_reg_info(void* arg, const regvm_reg_info* info)
{
    ((debugger*)arg)->reg_info(info);
}

void debugger::dump_var_info(void* arg, const regvm_var_info* info)
{
    ((debugger*)arg)->var_info(info);
}

