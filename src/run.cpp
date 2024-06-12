#include <regvm.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"

extern "C"
{

bool regvm_exec(struct regvm* vm, const code_t* start, int count, int64_t* exit)
{
    int rest = count;
    int offset = 0;
    const code_t* cur = start;
    while (rest > 0)
    {
        int next = 0;
        //TODO
        //if (exec_step(vm, cur, offset, rest, &next) == false)
        {
            //TODO : ERROR
            printf("\e[31m run ERROR at %d\e[0m\n", count - rest);
            return false;
        }
        if (next == 0)
        {
            return true;
        }

        cur += next;
        rest -= next;
        offset += next;
    }
    *exit = (int64_t)vm->reg.id(0);
    return true;
}

int regvm_exec_step(struct regvm* vm, const code_t* code, int max)
{
    int next = 0;
    //TODO
    //return (exec_step(vm, code, 0, max, &next) == false) ? 0 : next;
    return next;
}

}   //extern C

#undef UNSUPPORT_TYPE


