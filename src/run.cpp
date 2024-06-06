#include <run.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"

extern "C"
{

bool regvm_exe_one(struct regvm* vm, const struct code* inst)
{
    switch (inst->id)
    {
    case NOP:
        return true;
    case SET:
        vm->reg.set(inst->reg, inst);
        break;
    case STORE:
        if (inst->value.str == NULL)
        {
            vm->reg.store(inst->reg);
        }
        else
        {
            vm->reg.store(inst->reg, vm->ctx->add(inst, vm->reg.type(inst->reg)));
        }
        break;
    case LOAD:
        vm->reg.load(inst->reg, vm->ctx->get(inst->value.str));
        break;
    case BLOCK:
        switch (inst->type)
        {
        case 1:
            vm->ctx->enter_block();
            break;
        case 2:
            vm->ctx->leave_block();
            break;
        default:
            break;
        }
        break;
    default:
        return false;
    };
    return true;
}

bool regvm_exe_pages(struct regvm* vm, const int pages_count, const code_page* pages)
{
    return false;
}

}

