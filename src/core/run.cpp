#include <regvm.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"

extern "C"
{

bool regvm_exec(struct regvm* vm, const code_t* start, int count, int64_t* exit)
{
    if (vm->run(start, count) == false)
    {
        //TODO : ERROR
        //printf("\e[31m run ERROR at %d\e[0m\n", count - rest);
        return false;
    }
    auto e = vm->reg.id(0);
    if (e.type == 0)
    {
        return false;
    }
    *exit = (int64_t)e;
    return true;
}

int regvm_exec_step(struct regvm* vm, const code_t* code, int max)
{
    int next = 0;
    return (core::func::step(vm, code, 0, max, &next) == false) ? 0 : next;
}

int regvm_code_len(code_t code)
{
    int count = 1;
    switch (code.id)
    {
    case CODE_NOP:
        count += code.ex;
        break;
    case CODE_SETS:
        count += 1;
        break;
    case CODE_SETI:
        count += 2;
        break;
    case CODE_SETL:
        count += 4;
        break;
    case CODE_CALL:
        count += 4;
        break;
    default:
        break;
    }
    return count;
}
}   //extern C


