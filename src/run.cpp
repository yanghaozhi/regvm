#include <run.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"

extern "C"
{

struct regvm* regvm_init()
{
    auto vm = new regvm();
    return vm;
}

bool regvm_exit(struct regvm* vm)
{
    delete vm;
    return false;
}

bool regvm_exe_one(struct regvm* vm, const struct code* inst)
{
    switch (inst->id)
    {
    case NOP:
        return true;
    case SET:
        vm->reg.set(inst->reg, inst);
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
