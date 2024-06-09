#include <regvm.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"

static bool vm_set(struct regvm* vm, const code_t code, const int64_t value)
{
    auto& r = vm->reg.id(code.reg);
    r.set(value, code.ex);
    return true;
}

static bool vm_store(struct regvm* vm, const code_t code)
{
    auto& r = vm->reg.id(code.reg);
    if (code.ex == 0)
    {
        r.store();
    }
    else
    {
        auto& e = vm->reg.id(code.ex);
        if ((e.type & 0x07) != TYPE_STRING)
        {
            //TODO
            ERROR(ERR_TYPE_MISMATCH, "store name : %d", e.type);
        }
        else
        {
            var* v = vm->ctx->add(r.type, e.value.str);
            r.store(v);
        }
    }
    return true;
}

static bool vm_load(struct regvm* vm, const code_t code)
{
    auto& e = vm->reg.id(code.ex);
    auto& r = vm->reg.id(code.reg);
    return r.load(vm->ctx->get(e.value.str));
}

static bool vm_move(struct regvm* vm, const code_t code)
{
    auto& e = vm->reg.id(code.ex);
    auto& r = vm->reg.id(code.reg);
    r.store();
    r.set_from(NULL);
    r.value.uint = e.value.uint;
    r.type = e.type;
    return true;
}

static bool vm_clear(struct regvm* vm, const code_t code)
{
    auto& r = vm->reg.id(code.reg);
    r.store();
    r.set_from(NULL);
    r.value.uint = 0;
    r.type = code.ex;
    return true;
}

static bool vm_conv_impl(struct regvm* vm, regs::reg_v& r, int to)
{
    if (r.type == to) return true;

    switch (to)
    {
    case TYPE_UNSIGNED:
        r.value.uint = (uint64_t)r;
        break;
    case TYPE_SIGNED:
        r.value.sint = (int64_t)r;
        break;
    case TYPE_DOUBLE:
        r.value.dbl = (double)r;
        break;
    default:
        //TODO : ERROR
        return false;
    }

    r.set_from(NULL);
    r.type = to;

    return true;
}

static bool vm_conv(struct regvm* vm, const code_t code)
{
    auto& r = vm->reg.id(code.reg);
    return vm_conv_impl(vm, r, code.ex);
}

static bool vm_chg(struct regvm* vm, const code_t code)
{
    auto& r = vm->reg.id(code.reg);
    switch (code.ex)
    {
    case 0:
        r.value.uint = 0;
        return true;
    case 1:
        switch (r.type)
        {
        case TYPE_UNSIGNED:
        case TYPE_SIGNED:
            r.value.sint = 0 - r.value.sint;
            break;
        case TYPE_DOUBLE:
            r.value.dbl = 0 - r.value.dbl;
            break;
        default:
            return false;
        }
        return true;
    case 2:
        switch (r.type)
        {
        case TYPE_UNSIGNED:
        case TYPE_SIGNED:
            if (vm_conv_impl(vm, r, TYPE_DOUBLE) == false) return false;
            [[fallthrough]];
        case TYPE_DOUBLE:
            r.value.dbl = 1 / r.value.dbl;
            break;
        default:
            return false;
        }
        return true;
    case 3:
        if (r.type == TYPE_UNSIGNED)
        {
            r.value.uint = ~r.value.uint;
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

static int vm_jump(struct regvm* vm, const code_t* code, const code_t* start)
{
    auto& e = vm->reg.id(code->ex);
    switch (e.type)
    {
    case TYPE_SIGNED:
        return (int64_t)e;
    case TYPE_UNSIGNED:
        return (uint64_t)e - (code - start);
    default:
        return 0;
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
    {                                                   \
        auto& e = vm->reg.id(code->ex);                 \
        auto& r = vm->reg.id(code->reg);                \
        if (r.type == TYPE_UNSIGNED)                    \
        {                                               \
            r.value.uint op (uint64_t)e;                \
        }                                               \
        else                                            \
        {                                               \
            /* TODO : ERROR */                          \
        }                                               \
    }


#define NULL_CALL()

extern "C"
{

bool regvm_exec(struct regvm* vm, const code_t* code, int count, int64_t* exit)
{
    int rest = count;
    const code_t* cur = code;
    while (rest > 0)
    {
        int r = 0;
        switch (cur->id)
        {
        case CODE_EXIT:
            if (cur->ex == 0)
            {
                *exit = 0;
            }
            else
            {
                *exit = (int64_t)vm->reg.id(code->reg);
            }
            return true;
        case CODE_JUMP:
            r = vm_jump(vm, cur, code);
            break;
#define JUMP(i, cmp)                                                                    \
        case CODE_##i:                                                                  \
            r = ((int64_t)vm->reg.id(code->reg) cmp 0) ? vm_jump(vm, cur, code) : 1;    \
            break;
            JUMP(JZ, ==);
            JUMP(JNZ, !=);
            JUMP(JG, >);
            JUMP(JL, <);
            JUMP(JNG, <=);
            JUMP(JNL, >=);
#undef JUMP
        default:
            r = regvm_exec_step(vm, cur, rest);
            break;
        }

        if (r == 0)
        {
            //TODO : ERROR
            printf("\e[31m run ERROR at %d\e[0m\n", count - rest);
            return false;
        }

        cur += r;
        rest -= r;
    }
    *exit = (int64_t)vm->reg.id(0);
    return true;
}

int regvm_exec_step(struct regvm* vm, const code_t* code, int max)
{
    int read_codes = 1;
    switch (code->id)
    {
#define EXTRA_RUN(id, need, func, ...)                                      \
    case CODE_##id:                                                         \
        if (max < (need))                                                   \
        {                                                                   \
            ERROR(ERR_INST_TRUNC, "need at lease %d bytes", (need) << 1);   \
            return 0;                                                       \
        }                                                                   \
        func(__VA_ARGS__);                                                  \
        read_codes += (need);                                               \
        break;

        EXTRA_RUN(NOP, code->ex, NULL_CALL);
        EXTRA_RUN(SETS, 1, vm_set, vm, *code, *(int16_t*)&code[1]);
        EXTRA_RUN(SETI, 2, vm_set, vm, *code, *(int32_t*)&code[1]);
        EXTRA_RUN(SETL, 4, vm_set, vm, *code, *(int64_t*)&code[1]);

#undef EXTRA_RUN

#define CODE_RUN(id, func, ...)             \
    case CODE_##id:                         \
        func(__VA_ARGS__);                  \
        break;

        CODE_RUN(TRAP, vm->idt.call<regvm_irq_trap>, vm, IRQ_TRAP, code->reg, code->ex);
        CODE_RUN(STORE, vm_store, vm, *code);
        CODE_RUN(LOAD, vm_load, vm, *code);
        CODE_RUN(BLOCK, RUN_BLOCK, 0);
        CODE_RUN(MOVE, vm_move, vm, *code);
        CODE_RUN(CLEAR, vm_clear, vm, *code);

        CODE_RUN(AND, BITWISE, &=);
        CODE_RUN(OR, BITWISE, |=);
        CODE_RUN(XOR, BITWISE, ^=);
        CODE_RUN(CONV, vm_conv, vm, *code);
        CODE_RUN(CHG, vm_chg, vm, *code);

#undef CODE_RUN

#define FROM_REG vm->reg.id(code->ex)
#define DIRECT code->ex
#define CALC(i, op, val)                                \
    case CODE_##i:                                      \
        {                                               \
            auto& r = vm->reg.id(code->reg);            \
            switch (r.type)                             \
            {                                           \
            case TYPE_SIGNED:                           \
                r.value.sint op (int64_t)val;           \
                break;                                  \
            case TYPE_UNSIGNED:                         \
                r.value.uint op (uint64_t)val;          \
                break;                                  \
            case TYPE_DOUBLE:                           \
                r.value.dbl op (double)val;             \
                break;                                  \
            default:                                    \
                break;                                  \
            }                                           \
        }                                               \
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

    return read_codes;
}

//bool regvm_exe_pages(struct regvm* vm, const int pages_count, const code_page* pages)
//{
//    return false;
//}

}

