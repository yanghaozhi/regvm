#include <regvm.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"

static bool vm_store(struct regvm* vm, const code_t code)
{
    if (code.ex == 0)
    {
        vm->reg.store(code.reg);
    }
    else
    {
        if (vm->reg.types[code.ex] != TYPE_STRING)
        {
            //TODO
            ERROR(ERR_TYPE_MISMATCH, "store name : %d", vm->reg.types[code.ex]);
        }
        else
        {
            int type = vm->reg.types[code.reg];
            const char* name = vm->reg.values[code.ex].str;
            var* v = vm->ctx->add(type, name);
            vm->reg.store(code.reg, v);
            if (v != NULL)
            {
                return false;
            }
        }
    }
    return true;
}


extern "C"
{

int regvm_exe_one(struct regvm* vm, const code_t* code, int max)
{
    int read_bytes = 2;
    switch (code->id)
    {
    case CODE_NOP:
        read_bytes += (code->ex) << 1;
        break;
    case CODE_TRAP:
        //debug_info(vm, code);
        vm->idt.call<regvm_irq_trap>(vm, IRQ_TRAP, code->reg, code->ex);
        break;
    case CODE_SETS:
        vm->reg.set(*code, *(uint16_t*)&code[1]);
        read_bytes += 2;
        break;
    case CODE_SETI:
        vm->reg.set(*code, *(uint32_t*)&code[1]);
        read_bytes += 4;
        break;
    case CODE_SETL:
        vm->reg.set(*code, *(uint64_t*)&code[1]);
        read_bytes += 8;
        break;
    case CODE_STORE:
        vm_store(vm, *code);
        break;
    case CODE_LOAD:
        vm->reg.load(code->reg, vm->ctx->get(vm->reg.values[code->reg].str));
        break;
    case CODE_BLOCK:
        switch (code->ex)
        {
        case 0:
            vm->ctx->enter_block();
            break;
        case 1:
            vm->ctx->leave_block();
            break;
        default:
            //TODO
            //ERROR
            break;
        }
        break;
    default:
        return -1;
    };
    return read_bytes;
}

//bool regvm_exe_pages(struct regvm* vm, const int pages_count, const code_page* pages)
//{
//    return false;
//}

}

