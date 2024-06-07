#include <run.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"


static int debug_info(struct regvm* vm, const code8_t* code)
{
    trap_callback cb = (trap_callback)code->other;
    cb(vm, code->base.type, code->base.reg);
    return true;
}


extern "C"
{

int regvm_exe_one(struct regvm* vm, const code0_t* code)
{
    int read_bytes = 2;
    switch (code->base.id)
    {
    case NOP:
        return true;
    case TRAP:
        debug_info(vm, ((const code8_t*)code));
        read_bytes += 8;
        break;
    case SET2:
        vm->reg.set(code->base, ((const code2_t*)code)->num);
        read_bytes += 2;
        break;
    case SET4:
        vm->reg.set(code->base, ((const code4_t*)code)->num);
        read_bytes += 4;
        break;
    case SET8:
        vm->reg.set(code->base, ((const code8_t*)code)->num);
        read_bytes += 8;
        break;
    case STORE:
        vm->reg.store(code->base.reg);
        break;
    case STORE8:
        {
            code8_t c = *(code8_t*)code;
            c.base.type = vm->reg.type(code->base.reg);
            vm->reg.store(code->base.reg, vm->ctx->add(&c));
        }
        read_bytes += 8;
        break;
    case LOAD:
        vm->reg.load(code->base.reg, vm->ctx->get(((code8_t*)code)->str));
        read_bytes += 8;
        break;
    case BLOCK:
        switch (code->base.type)
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
        return -1;
    };
    return read_bytes;
}

bool regvm_exe_pages(struct regvm* vm, const int pages_count, const code_page* pages)
{
    return false;
}

}

