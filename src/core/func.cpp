#include <regvm.h>
#include <irq.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "ext.h"

#include <log.h>

using namespace core;



extern vm_sub_op_t  CHG_OPS[16];


inline bool vm_conv_impl(struct regvm* vm, reg::v& r, int to);


inline int step(struct regvm* vm, code_t inst, int offset, int max, const void* extra)
{
    int next = 1;

#define STEP_ERROR(e, fmt, ...) VM_ERROR(e, inst, offset, fmt, ##__VA_ARGS__);

    int code = inst.id;

    LOGT("%d : code %8s - 0x%02X - 0x%02X", offset, CODE_NAME(code), code, inst.value);

    switch (code)
    {
#define CALC(i, op)                                                         \
    case CODE_##i:                                                          \
        {                                                                   \
            const auto& a = vm->reg.id(inst.a);                             \
            const auto& b = vm->reg.id(inst.b);                             \
            auto& r = vm->reg.id(inst.c);                                   \
            int t = (a.type > b.type) ? a.type : b.type;                    \
            vm_conv_impl(vm, r, t);                                         \
            switch (t)                                                      \
            {                                                               \
            case TYPE_SIGNED:                                               \
                r.value.sint = (int64_t)a op (int64_t)b;                    \
                break;                                                      \
            case TYPE_UNSIGNED:                                             \
                r.value.uint = (uint64_t)a op (uint64_t)b;                  \
                break;                                                      \
            case TYPE_DOUBLE:                                               \
                r.value.dbl = (double)a op (double)b;                       \
                break;                                                      \
            default:                                                        \
                UNSUPPORT_TYPE(#i, t, inst, offset);                        \
                break;                                                      \
            }                                                               \
        }                                                                   \
        break;
        CALC(ADD, +);
        CALC(SUB, -);
        CALC(MUL, *);
        CALC(DIV, /);
#undef CALC

#define ONLY_UNSIGNED(i, op)                                                \
    case CODE_##i:                                                          \
        {                                                                   \
            const auto& a = vm->reg.id(inst.a);                             \
            const auto& b = vm->reg.id(inst.b);                             \
            auto& r = vm->reg.id(inst.c);                                   \
            vm_conv_impl(vm, r, TYPE_UNSIGNED);                             \
            if ((a.type == TYPE_UNSIGNED) && (b.type == TYPE_UNSIGNED))     \
            {                                                               \
                r.value.uint = (uint64_t)a op (uint64_t)b;                  \
            }                                                               \
            else                                                            \
            {                                                               \
                UNSUPPORT_TYPE(#i, r.type, inst, offset);                   \
            }                                                               \
        }                                                                   \
        break;
        ONLY_UNSIGNED(AND, &);
        ONLY_UNSIGNED(OR, |);
        ONLY_UNSIGNED(XOR, ^);
#undef ONLY_UNSIGNED

#define ONLY_INTEGER(i, op)                                                 \
    case CODE_##i:                                                          \
        {                                                                   \
            const auto& a = vm->reg.id(inst.a);                             \
            const auto& b = vm->reg.id(inst.b);                             \
            auto& r = vm->reg.id(inst.c);                                   \
            vm_conv_impl(vm, r, a.type);                                    \
            switch (r.type)                                                 \
            {                                                               \
            case TYPE_SIGNED:                                               \
                r.value.sint = (int64_t)a op (int64_t)b;                    \
                break;                                                      \
            case TYPE_UNSIGNED:                                             \
                r.value.uint = (uint64_t)a op (uint64_t)b;                  \
                break;                                                      \
            default:                                                        \
                UNSUPPORT_TYPE(#i, r.type, inst, offset);                   \
                break;                                                      \
            }                                                               \
        }                                                                   \
        break;
        ONLY_INTEGER(SHL, <<);
        ONLY_INTEGER(SHR, >>);
        ONLY_INTEGER(MOD, %);
#undef ONLY_INTEGER

#define B_LITERALLY(i, op)                                                  \
    case CODE_##i:                                                          \
        {                                                                   \
            const auto& a = vm->reg.id(inst.a);                             \
            const int b = inst.bs;                                          \
            auto& r = vm->reg.id(inst.c);                                   \
            vm_conv_impl(vm, r, a.type);                                    \
            switch (r.type)                                                 \
            {                                                               \
            case TYPE_SIGNED:                                               \
                r.value.sint = (int64_t)a op b;                             \
                break;                                                      \
            case TYPE_UNSIGNED:                                             \
                r.value.uint = (uint64_t)a op b;                            \
                break;                                                      \
            default:                                                        \
                UNSUPPORT_TYPE(#i, r.type, inst, offset);                   \
                break;                                                      \
            }                                                               \
        }                                                                   \
        break;
        B_LITERALLY(INC, +);
        //B_LITERALLY(SHR, >>);
        //B_LITERALLY(MOD, %);
#undef B_LITERALLY

#define JUMPS(i, cmp)                                                       \
    case CODE_##i:                                                          \
        {                                                                   \
            const auto& a = vm->reg.id(inst.a);                             \
            const auto& b = vm->reg.id(inst.b);                             \
            const auto& dest = vm->reg.id(inst.c);                          \
            int t = (a.type > b.type) ? a.type : b.type;                    \
            switch (t)                                                      \
            {                                                               \
            case TYPE_SIGNED:                                               \
                return ((int64_t)a cmp (int64_t)b) ? (int64_t)dest : 1;     \
            case TYPE_UNSIGNED:                                             \
                return ((uint64_t)a cmp (uint64_t)b) ? (int64_t)dest : 1;   \
            case TYPE_DOUBLE:                                               \
                return ((double)a cmp (double)b) ? (int64_t)dest : 1;       \
            default:                                                        \
                UNSUPPORT_TYPE(#i, t, inst, offset);                        \
                break;                                                      \
            }                                                               \
        }                                                                   \
        break;
        JUMPS(JEQ, ==);
        JUMPS(JNE, !=);
        JUMPS(JGT, >);
        JUMPS(JGE, >=);
        JUMPS(JLT, <);
        JUMPS(JLE, <=);
#undef JUMPS

#define WRITE(i, ...)                                                       \
    case CODE_##i:                                                          \
        {                                                                   \
            auto& r = vm->reg.id(inst.a);                                   \
            auto& e = vm->reg.id(inst.b);                                   \
            return r.write(__VA_ARGS__);                                    \
        }
        WRITE(MOVE, e.value.uint, e.type, true);
        WRITE(TYPE, e.type, TYPE_SIGNED, true);
#undef WRITE

#define SUB_OPS(i, op)                                                      \
    case CODE_##i:                                                          \
        {                                                                   \
            auto f = i##_OPS[inst.op];                                      \
            if (unlikely(f == NULL))                                        \
            {                                                               \
                UNSUPPORT_TYPE(#i, inst.op, inst, offset);                  \
                return 0;                                                   \
            }                                                               \
            return f(vm, inst.a, inst.b, inst.c, offset);                   \
        }                                                                   \
        break;
        SUB_OPS(CHG, c);
#undef SUBS

    case CODE_CMP:
        switch (inst.c)
        {
#define CMP(k, cmp)                             \
    case k:                                     \
        vm->reg.id(inst.a).write(((int64_t)vm->reg.id(inst.b) cmp 0), TYPE_SIGNED, true);  \
        return 1;
            CMP(0, ==);
            CMP(1, !=);
            CMP(2, >);
            CMP(3, >=);
            CMP(4, <);
            CMP(5, <=);
#undef CMP
        }
        break;
    case CODE_EXIT:
        vm->exit_code = ((int)inst.a2 != -1) ? inst.b2 : (int64_t)vm->reg.id(inst.a2);
        vm->exit = true;
        return 0;
    case CODE_RET:
        return 0;
    default:
        if (likely(code >= CODE_TRAP))
        {
            auto f = vm->ops[code - CODE_TRAP];
            if (likely(f != NULL))
            {
                int r = f(vm, inst, offset, extra);
                if (unlikely(r == 0))
                {
                    VM_ERROR(ERR_RUNTIME, inst, offset, "run code ERROR : %8s - 0x%02X", CODE_NAME(code), inst.value);
                }
                return r;
            }
        }

        VM_ERROR(ERR_INVALID_CODE, inst, offset, "invalid code : %8s - 0x%02X", CODE_NAME(code), inst.value);
        fprintf(stderr, "code %d is NOT SUPPORT YET", code);
        return 0;
    };

    if (likely(vm->fatal == false))
    {
        return next;
    }
    else
    {
        return 0;
    }
}

inline bool vm_conv_impl(struct regvm* vm, reg::v& r, int to)
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

bool vm_conv_type(struct regvm* vm, reg::v& r, int to)
{
    return vm_conv_impl(vm, r, to);
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
        int next = step(vm, *cur, offset, rest, cur + 1);
        if (unlikely(next == 0))
        {
            return ! vm->fatal;
        }

        cur += next;
        rest -= next;
        offset += next;
    }

    if (rest < 0)
    {
        VM_ERROR(ERR_RUNTIME, code_t{0}, offset, "run to UNEXPECT POSISTION : %d - %lld", rest, offset);
    }

    auto& r = vm->reg.id(0);
    vm->exit_code = (r.type == TYPE_NULL) ? 0 : r.value.sint;
    return true;
}

bool func::one_step(struct regvm* vm, const code_t code, int max, int* next, const void* extra)
{
    return step(vm, code, 0, max, extra);
}

#undef UNSUPPORT_TYPE

