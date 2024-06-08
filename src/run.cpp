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
        }
    }
    return true;
}

static bool vm_conv(struct regvm* vm, const code_t code)
{
    return true;
}

static bool vm_chg(struct regvm* vm, const code_t code)
{
    switch (code.ex)
    {
    case 0:
        vm->reg.values[code.reg].uint = 0;
        return true;
    case 1:
        switch (vm->reg.types[code.reg])
        {
        case TYPE_UNSIGNED:
        case TYPE_SIGNED:
            vm->reg.values[code.reg].sint = 0 - vm->reg.values[code.reg].sint;
            break;
        case TYPE_DOUBLE:
            vm->reg.values[code.reg].dbl = 0 - vm->reg.values[code.reg].dbl;
            break;
        default:
            return false;
        }
        return true;
    case 2:
        switch (vm->reg.types[code.reg])
        {
        case TYPE_UNSIGNED:
        case TYPE_SIGNED:
            vm->reg.values[code.reg].dbl = 0 / vm->reg.values[code.reg].conv(vm->reg.types[code.ex], vm->reg.values[code.reg].dbl);
            return true;
        case TYPE_DOUBLE:
            vm->reg.values[code.reg].dbl = 0 / vm->reg.values[code.reg].dbl;
            break;
        default:
            return false;
        }
        return true;
    case 3:
        if (vm->reg.types[code.reg] == TYPE_UNSIGNED)
        {
            vm->reg.values[code.reg].uint = ~vm->reg.values[code.reg].uint;
            return true;
        }
        else
        {
            return false;
        }
    default:
        return false;
    }
}

#define RUN_BLOCK(t)                \
    switch (code->ex)               \
    {                               \
    case 0:                         \
        vm->ctx->enter_block();     \
        break;                      \
    case 1:                         \
        vm->ctx->leave_block();     \
        break;                      \
    default:                        \
        /* TODO : ERROR */          \
        break;                      \
    }

#define BITWISE(op)                                     \
    if (vm->reg.types[code->reg] == TYPE_UNSIGNED)      \
    {                                                   \
    vm->reg.values[code->reg].uint op vm->reg.values[code->ex].conv(vm->reg.types[code->ex], vm->reg.values[code->ex].sint);    \
    }                                                   \
    else                                                \
    {                                                   \
        /* TODO : ERROR */                              \
    }

#define FROM_REG(t) vm->reg.values[code->ex].conv(vm->reg.types[code->ex], vm->reg.values[code->ex].t)

#define DIRECT(t) code->ex

#define NULL_CALL()

extern "C"
{

int regvm_exe_one(struct regvm* vm, const code_t* code, int max_bytes)
{
    if (max_bytes < 2)
    {
        ERROR(ERR_INST_TRUNC, "need at lease 2 bytes for each inst");
        return 0;
    }

    int read_bytes = 2;
    switch (code->id)
    {
#define EXTRA_RUN(id, need, func, ...)                                  \
    case CODE_##id:                                                     \
        if (max_bytes - 2 < (need))                                     \
        {                                                               \
            ERROR(ERR_INST_TRUNC, "need at lease %d bytes", (need));    \
            return 0;                                                   \
        }                                                               \
        func(__VA_ARGS__);                                              \
        read_bytes += (need);                                           \
        break;

        EXTRA_RUN(NOP, (code->ex) << 1, NULL_CALL);
        EXTRA_RUN(SETS, 2, vm->reg.set, *code, *(uint16_t*)&code[1]);
        EXTRA_RUN(SETI, 4, vm->reg.set, *code, *(uint32_t*)&code[1]);
        EXTRA_RUN(SETL, 8, vm->reg.set, *code, *(uint64_t*)&code[1]);

#undef EXTRA_RUN

#define CODE_RUN(id, func, ...)             \
    case CODE_##id:                         \
        func(__VA_ARGS__);                  \
        break;

        CODE_RUN(TRAP, vm->idt.call<regvm_irq_trap>, vm, IRQ_TRAP, code->reg, code->ex);
        CODE_RUN(STORE, vm_store, vm, *code);
        CODE_RUN(LOAD, vm->reg.load, code->reg, vm->ctx->get(vm->reg.values[code->reg].str));
        CODE_RUN(BLOCK, RUN_BLOCK, 0);

        CODE_RUN(AND, BITWISE, &=);
        CODE_RUN(OR, BITWISE, |=);
        CODE_RUN(XOR, BITWISE, ^=);
        CODE_RUN(CONV, vm_conv, vm, *code);
        CODE_RUN(CHG, vm_chg, vm, *code);

#undef CODE_RUN

#define CALC(id, op, val)                                   \
    case CODE_##id:                                         \
        switch (vm->reg.types[code->reg])                   \
        {                                                   \
        case TYPE_SIGNED:                                   \
            vm->reg.values[code->reg].sint op val(sint);    \
            break;                                          \
        case TYPE_UNSIGNED:                                 \
            vm->reg.values[code->reg].uint op val(sint);    \
            break;                                          \
        case TYPE_DOUBLE:                                   \
            vm->reg.values[code->reg].dbl op val(sint);     \
            break;                                          \
        default:                                            \
            break;                                          \
        }                                                   \
        break;

        CALC(INC, +=, DIRECT);
        CALC(DEC, -=, DIRECT);
        CALC(ADD, +=, FROM_REG);
        CALC(SUB, -=, FROM_REG);
        CALC(MUL, *=, FROM_REG);
        CALC(DIV, /=, FROM_REG);

#undef CALC

    default:
        fprintf(stderr, "code %d is NOT SUPPORT YET", code->id);
        return 0;
    };

    return read_bytes;
}

//bool regvm_exe_pages(struct regvm* vm, const int pages_count, const code_page* pages)
//{
//    return false;
//}

}

