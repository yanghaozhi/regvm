#include <regvm.h>
#include <irq.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"

#define UNSUPPORT_TYPE(op, t, c, o) ERROR(ERR_TYPE_MISMATCH, c, o, "UNSUPPORT %s value type : %d", op, t); 

using namespace core;

static bool vm_set(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    auto& r = vm->reg.id(code.reg);
    if ((code.ex == TYPE_STRING) && (value & 0x01))
    {
        //auto it = vm->strs.find(value);
        //if (it == vm->strs.end())
        //{
        //    ERROR(ERR_STRING_RELOCATE, code, offset, "need to relocate string : %ld", value);
        //    return false;
        //}
        //value = (intptr_t)it->second;
        auto& it = vm->idt.isrs[IRQ_STR_RELOCATE];
        if (it.func == NULL)
        {
            ERROR(ERR_STRING_RELOCATE, code, offset, "need to relocate string : %ld", value);
            return false;
        }
        value = it.call(vm, IRQ_STR_RELOCATE, code, offset, (void*)value);
        if (value == 0)
        {
            ERROR(ERR_STRING_RELOCATE, code, offset, "relocate string : %ld ERROR", value);
            return false;
        }
    }
    return r.set(value, code.ex);
}

static bool vm_move(struct regvm* vm, const code_t code)
{
    auto& e = vm->reg.id(code.ex);
    auto& r = vm->reg.id(code.reg);
    r.clear();
    r.value.uint = e.value.uint;
    r.type = e.type;
    return true;
}

static bool vm_clear(struct regvm* vm, const code_t code)
{
    auto& r = vm->reg.id(code.reg);
    r.clear();
    r.value.uint = 0;
    r.type = code.ex;
    return true;
}

static bool vm_conv_impl(struct regvm* vm, reg::v& r, int to)
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
        return false;
    }

    r.set_from(NULL);
    r.type = to;

    return true;
}

static bool vm_conv(struct regvm* vm, const code_t code, int offset)
{
    auto& r = vm->reg.id(code.reg);
    if (vm_conv_impl(vm, r, code.ex) == false)
    {
        UNSUPPORT_TYPE("conv", code.ex, code, offset);
        return false;
    }
    return true;
}

static bool vm_type(struct regvm* vm, const code_t code, int offset)
{
    auto& r = vm->reg.id(code.reg);
    auto& e = vm->reg.id(code.ex);
    r.clear();
    r.value.uint = e.type;
    r.type = TYPE_UNSIGNED;
    return true;
}

static bool vm_chg(struct regvm* vm, const code_t code, int offset)
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
            UNSUPPORT_TYPE("chg", r.type, code, offset);
            return false;
        }
        return true;
    case 2:
        switch (r.type)
        {
        case TYPE_UNSIGNED:
        case TYPE_SIGNED:
            if (vm_conv_impl(vm, r, TYPE_DOUBLE) == false)
            {
                UNSUPPORT_TYPE("chg", r.type, code, offset);
                return false;
            }
            [[fallthrough]];
        case TYPE_DOUBLE:
            r.value.dbl = 1 / r.value.dbl;
            break;
        default:
            UNSUPPORT_TYPE("chg", r.type, code, offset);
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
            UNSUPPORT_TYPE("chg", r.type, code, offset);
            return false;
        }
    default:
        UNSUPPORT_TYPE("chg", code.ex, code, offset);
        return false;
    }
}


static int vm_jump(struct regvm* vm, const code_t code, int offset)
{
    auto& e = vm->reg.id(code.ex);
    switch (e.type)
    {
    case TYPE_SIGNED:
        return (int64_t)e;
    case TYPE_ADDR:
        return e.conv_i(TYPE_UNSIGNED) - offset;
    default:
        return 0;
    }
}



#define NULL_CALL() true


bool func::step(struct regvm* vm, const code_t* code, int offset, int max, int* next)
{
    *next = 1;

#define STEP_ERROR(e, fmt, ...) ERROR(e, *code, offset, fmt, ##__VA_ARGS__);
#define CALL(f, ...)                                                    \
    if (f(__VA_ARGS__) == false)                                        \
    {                                                                   \
        STEP_ERROR(ERR_RUNTIME, "run code %u-%u-%u at %d ERROR",        \
                code->id, code->reg, code->ex, offset);                 \
    }

    switch (code->id)
    {
#define EXTRA_RUN(id, need, func, ...)                                          \
    case CODE_##id:                                                             \
        if (max < (need))                                                       \
        {                                                                       \
            STEP_ERROR(ERR_INST_TRUNC, "need at lease %d bytes", (need) << 1);  \
            return 0;                                                           \
        }                                                                       \
        CALL(func, ##__VA_ARGS__);                                              \
        *next += (need);                                                        \
        break;
        EXTRA_RUN(NOP, code->ex, NULL_CALL);
        EXTRA_RUN(SETS, 1, vm_set, vm, *code, offset, *(int16_t*)&code[1]);
        EXTRA_RUN(SETI, 2, vm_set, vm, *code, offset, *(int32_t*)&code[1]);
        EXTRA_RUN(SETL, 4, vm_set, vm, *code, offset, *(int64_t*)&code[1]);
#undef EXTRA_RUN

#define CODE_RUN(id, func, ...)             \
    case CODE_##id:                         \
        CALL(func, ##__VA_ARGS__);          \
        break;
        CODE_RUN(STORE, vm->handlers.vm_store, vm, *code, offset, -1);
        CODE_RUN(LOAD, vm->handlers.vm_load, vm, *code, offset, -1);
        CODE_RUN(BLOCK, vm->handlers.vm_block, vm, *code, offset, vm->call_stack->id);
        CODE_RUN(MOVE, vm_move, vm, *code);
        CODE_RUN(CLEAR, vm_clear, vm, *code);
        CODE_RUN(CONV, vm_conv, vm, *code, offset);
        CODE_RUN(TYPE, vm_type, vm, *code, offset);
        CODE_RUN(CHG, vm_chg, vm, *code, offset);
#undef CODE_RUN

#define FROM_REG vm->reg.id(code->ex)
#define DIRECT code->ex
#define CALC(i, op, val)                                            \
    case CODE_##i:                                                  \
        {                                                           \
            auto& r = vm->reg.id(code->reg);                        \
            switch (r.type)                                         \
            {                                                       \
            case TYPE_SIGNED:                                       \
                r.value.sint op (int64_t)val;                       \
                break;                                              \
            case TYPE_UNSIGNED:                                     \
                r.value.uint op (uint64_t)val;                      \
                break;                                              \
            case TYPE_DOUBLE:                                       \
                r.value.dbl op (double)val;                         \
                break;                                              \
            default:                                                \
                UNSUPPORT_TYPE("calc", r.type, *code, offset);      \
                break;                                              \
            }                                                       \
        }                                                           \
        break;
        CALC(INC, +=, DIRECT);
        CALC(DEC, -=, DIRECT);
        CALC(ADD, +=, FROM_REG);
        CALC(SUB, -=, FROM_REG);
        CALC(MUL, *=, FROM_REG);
        CALC(DIV, /=, FROM_REG);
#undef CALC

#define BITWISE(i, op)                                              \
    case CODE_##i:                                                  \
        {                                                           \
            auto& e = vm->reg.id(code->ex);                         \
            auto& r = vm->reg.id(code->reg);                        \
            if (r.type == TYPE_UNSIGNED)                            \
            {                                                       \
                r.value.uint op (uint64_t)e;                        \
            }                                                       \
            else                                                    \
            {                                                       \
                UNSUPPORT_TYPE("bitwise", r.type, *code, offset);   \
            }                                                       \
        }                                                           \
        break;
        BITWISE(AND, &=);
        BITWISE(OR, |=);
        BITWISE(XOR, ^=);
#undef BITWISE

#define SHIFT(i, op)                                                \
    case CODE_##i:                                                  \
        {                                                           \
            auto& e = vm->reg.id(code->ex);                         \
            auto& r = vm->reg.id(code->reg);                        \
            switch (r.type)                                         \
            {                                                       \
            case TYPE_SIGNED:                                       \
                r.value.sint op (uint64_t)e;                        \
                break;                                              \
            case TYPE_UNSIGNED:                                     \
                r.value.uint op (uint64_t)e;                        \
                break;                                              \
            default:                                                \
                UNSUPPORT_TYPE("shift", r.type, *code, offset);     \
                break;                                              \
            }                                                       \
        }                                                           \
        break;
        SHIFT(SHL, <<=);
        SHIFT(SHR, >>=);
#undef BITWISE

#define JUMP(i, cmp)                                                                        \
    case CODE_##i:                                                                          \
        *next = ((int64_t)vm->reg.id(code->reg) cmp 0) ? vm_jump(vm, *code, offset) : 1;    \
        break;
        JUMP(JZ, ==);
        JUMP(JNZ, !=);
        JUMP(JG, >);
        JUMP(JL, <);
        JUMP(JNG, <=);
        JUMP(JNL, >=);
#undef JUMP

    case CODE_TRAP:
        *next = vm->idt.call(vm, IRQ_TRAP, *code, offset, &vm->call_stack->running->src, *next);
        break;
    case CODE_JUMP:
        *next = vm_jump(vm, *code, offset);
        break;
    case CODE_EXIT:
        vm->exit_code = (code->ex == 0) ? 0 : (int64_t)vm->reg.id(code->reg);
        vm->exit = true;
        [[fallthrough]];
    case CODE_RET:
        *next = 0;
        return true;
    case CODE_CALL:
        {
            auto& r = vm->reg.id(code->reg);
            if ((vm->call(r, *code, offset) == false) || (vm->fatal == true))
            {
                return false;
            }
            if (vm->exit == true)
            {
                return true;
            }
        }
        break;
    default:
        ERROR(ERR_INVALID_CODE, *code, offset, "invalid code : %u - %u - %u", code->id, code->reg, code->ex);
        fprintf(stderr, "code %d is NOT SUPPORT YET", code->id);
        return false;
    };

#undef CALL

    return !vm->fatal;
}

func::func(const code_t* p, int64_t c, int64_t e, int64_t s, int32_t i, const regvm_src_location* l) :
    id(i), codes(p), count(c), entry(e), size(s)
{
    if (l == NULL)
    {
        src.line = 0;
        src.file = NULL;
        src.func = NULL;
    }
    else
    {
        src = *l;
    }
}

bool func::run(struct regvm* vm, int64_t start)
{
    int64_t offset = (start >= 0) ? start : entry;
    int rest = size;
    const code_t* cur = codes + offset;
    while (rest > 0)
    {
        int next = 0;
        if (step(vm, cur, offset, rest, &next) == false)
        {
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
    auto& r = vm->reg.id(0);
    vm->exit_code = (r.type == TYPE_NULL) ? 0 : (int64_t)r;
    return true;
}

#undef UNSUPPORT_TYPE

