#include "debugger.h"

#include <irq.h>

#include <stdio.h>


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

void debugger::reg_info(reg_arg* arg, const regvm_reg_info* info)
{
    switch ((intptr_t)info)
    {
    case 0:
        printf("\e[33m id\ttype\tref\tvar\tvalue \e[0m\n");
        break;
    case -1:
        break;
    default:
        if ((arg->reg_id < 0) || (arg->reg_id == info->id))
        {
            printf(" %d\t%d\t%d\t%p\t", info->id, info->type, info->ref, info->from);
            regvm_debug_uvalue_print(info->type, info->value);
            printf("\n");
        }
        break;
    }
}

void debugger::var_info(var_arg* arg, const regvm_var_info* info)
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

int64_t debugger::debug_trap(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
{
    auto d = (debugger*)arg;

    printf("%d :\n", offset);
    reg_arg r = {d, -1};
    var_arg v = {d};
    switch (code.ex)
    {
    case 0: //all regs
        if ((d->mode & REG) != 0)
        {
            regvm_debug_reg_callback(vm, dump_reg_info, &r);
        }
        break;
    case 1: //1 arg
        if ((d->mode & REG) != 0)
        {
            r.reg_id = code.reg;
            regvm_debug_reg_callback(vm, dump_reg_info, &r);
        }
        break;
    case 2:
        if ((d->mode & VAR) != 0)
        {
            regvm_debug_var_callback(vm, dump_var_info, &v);
        }
        break;
    default:
        break;
    }
    return true;
}

void debugger::dump_reg_info(void* arg, const regvm_reg_info* info)
{
    auto p = (reg_arg*)arg;
    p->self->reg_info(p, info);
}

void debugger::dump_var_info(void* arg, const regvm_var_info* info)
{
    auto p = (var_arg*)arg;
    p->self->var_info(p, info);
}

