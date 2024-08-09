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
inline int vm_jcmp(struct regvm* vm, int a, int b, int c, const void* extra);
inline int vm_equivalent(struct regvm* vm, int a, int b, int c);


inline int step(struct regvm* vm, code_t inst, int offset, int max, const void* extra)
{
    int next = 1;

#define STEP_ERROR(e, fmt, ...) VM_ERROR(e, inst, offset, fmt, ##__VA_ARGS__);

    const int code = inst.id;

    LOGT("%4d : code %8s - 0x%02X - %d - %d - %d", offset, CODE_NAME(code), code, inst.a, inst.b, inst.c);

    switch (code)
    {
#define CALC(i, op)                                                         \
    case CODE_##i:                                                          \
        {                                                                   \
            auto& r = vm->reg.id(inst.a);                                   \
            const auto& b = vm->reg.id(inst.b);                             \
            const auto& c = vm->reg.id(inst.c);                             \
            int t = (b.type > c.type) ? b.type : c.type;                    \
            vm_conv_impl(vm, r, t);                                         \
            switch (t)                                                      \
            {                                                               \
            case TYPE_SIGNED:                                               \
                r.value.sint = (int64_t)b op (int64_t)c;                    \
                break;                                                      \
            case TYPE_UNSIGNED:                                             \
                r.value.uint = (uint64_t)b op (uint64_t)c;                  \
                break;                                                      \
            case TYPE_DOUBLE:                                               \
                r.value.dbl = (double)b op (double)c;                       \
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
            auto& r = vm->reg.id(inst.a);                                   \
            const auto& b = vm->reg.id(inst.b);                             \
            const auto& c = vm->reg.id(inst.c);                             \
            vm_conv_impl(vm, r, TYPE_UNSIGNED);                             \
            if (likely((b.type == TYPE_UNSIGNED) && (c.type == TYPE_UNSIGNED)))    \
            {                                                               \
                r.value.uint = (uint64_t)b op (uint64_t)c;                  \
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
            auto& r = vm->reg.id(inst.a);                                   \
            const auto& b = vm->reg.id(inst.b);                             \
            const auto& c = vm->reg.id(inst.c);                             \
            vm_conv_impl(vm, r, b.type);                                    \
            switch (r.type)                                                 \
            {                                                               \
            case TYPE_SIGNED:                                               \
                r.value.sint = (int64_t)b op (int64_t)c;                    \
                break;                                                      \
            case TYPE_UNSIGNED:                                             \
                r.value.uint = (uint64_t)b op (uint64_t)c;                  \
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
    case CODE_JUMP:
        return inst.a3;
    case CODE_JCMP:
        return vm_jcmp(vm, inst.a, inst.b, inst.c, extra);
    case CODE_CALC:
        return vm_equivalent(vm, inst.a, inst.b, inst.c);
    case CODE_EXIT:
        //vm->exit_code = ((unsigned int)inst.a2 != 255) ? inst.b2 : (int64_t)vm->reg.id(inst.a2);
        vm->exit_code = inst.b2;
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

    if (likely(r.type != 0))
    {
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
    }

    r.set_from(NULL);
    r.type = to;

    return true;
}

inline int vm_jcmp(struct regvm* vm, int a, int b, int c, const void* extra)
{
    code_t* p = (code_t*)extra;
    if (unlikely(p->id != CODE_DATA))
    {
        return 0;
    }
    const int dest = p->a3;
    const int next = 2;

    const int m1 = 0x80;
    const int m2 = 0x40;
    const int v1 = a;
    const int v2 = b;
    const auto& r1 = vm->reg.id(a);
    const auto& r2 = vm->reg.id(b);
    const int t1 = ((c & 0x80) != 0) ? (int)TYPE_SIGNED : r1.type;
    const int t2 = ((c & 0x40) != 0) ? (int)TYPE_SIGNED : r2.type;
    const int t = (t1 > t2) ? t1 : t2;

#define CMP_VAL(type, idx) (((c & m##idx) != 0) ? (type)v##idx : (type)r##idx)
#define CMP_JUMP(type, cmp) return (CMP_VAL(type, 1) cmp CMP_VAL(type, 2)) ? dest : next;
    switch (c & 0x0F)
    {
#define CMP_TYPE(k, cmp)                \
    case k:                             \
        switch (t)                      \
        {                               \
        case TYPE_SIGNED:               \
            CMP_JUMP(int64_t, cmp);     \
        case TYPE_UNSIGNED:             \
            CMP_JUMP(uint64_t, cmp);    \
        case TYPE_DOUBLE:               \
            CMP_JUMP(double, cmp);      \
        default:                        \
            return 0;                   \
        }                               \
        break;
        CMP_TYPE(0, ==);
        CMP_TYPE(1, !=);
        CMP_TYPE(2, > );
        CMP_TYPE(3, >=);
        CMP_TYPE(4, < );
        CMP_TYPE(5, <=);
#undef CMP_TYPE
    }
#undef CMP_JUMP
#undef CMP_VAL

    return 0;
}

inline int vm_equivalent(struct regvm* vm, int a, int b, int c)
{
    auto& v = vm->reg.id(a);

#define ALL(k, op)                                                      \
    case k:                                                             \
        switch (v.type)                                                 \
        {                                                               \
        case TYPE_SIGNED:                                               \
            v.value.sint op b;                                          \
            break;                                                      \
        case TYPE_UNSIGNED:                                             \
            v.value.uint op b;                                          \
            break;                                                      \
        case TYPE_DOUBLE:                                               \
            v.value.dbl op b;                                           \
            break;                                                      \
        default:                                                        \
            return 0;                                                   \
        }                                                               \
        break;

#define INT(k, op)                                                      \
    case k:                                                             \
        switch (v.type)                                                 \
        {                                                               \
        case TYPE_SIGNED:                                               \
            v.value.sint op b;                                          \
            break;                                                      \
        case TYPE_UNSIGNED:                                             \
            v.value.uint op b;                                          \
            break;                                                      \
        default:                                                        \
            return 0;                                                   \
        }                                                               \
        break;
    switch (c)
    {
        ALL(0, +=);
        ALL(1, -=);
        ALL(2, *=);
        ALL(3, /=);
        INT(4, %=);
        INT(5, <<=);
        INT(6, >>=);
    default:
        return 0;
    }
#undef INT
#undef ALL
    return 1;
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
            VM_ERROR(ERR_RUNTIME, *cur, offset, "run code ERROR at %lld : %s - %d - %d - %d", offset, CODE_NAME(cur->id), cur->a, cur->b, cur->c);
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

