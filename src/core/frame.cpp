#include <regvm.h>
#include <irq.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "ext.h"
#include "frame.h"

#include <log.h>


using namespace core;



extern vm_sub_op_t  CHG_OPS[16];



inline int vm_set(regvm* vm, int a, int b, int c, const void* extra)
{
    uint64_t v = c;
    int shift = 8;
    int next = 1;
    code_t* p = (code_t*)extra;
    while (p->id == CODE_DATA)
    {
        uint64_t vv = (unsigned int)p->a3 & 0xFFFFFF;
        v += (vv << shift);
        shift += 24;
        next += 1;
        p += 1;
    }
    auto& r = vm->reg.id(a);
    r.write(v, b, (b != r.type));
    return next;
    //if ((ex == TYPE_STRING) && (value & 0x01))
    //{
    //    //auto it = vm->strs.find(value);
    //    //if (it == vm->strs.end())
    //    //{
    //    //    VM_ERROR(ERR_STRING_RELOCATE, code, reg, ex, offset, "need to relocate string : %ld", value);
    //    //    return false;
    //    //}
    //    //value = (intptr_t)it->second;
    //    auto& it = vm->idt.isrs[IRQ_STR_RELOCATE];
    //    if (it.func == NULL)
    //    {
    //        VM_ERROR(ERR_STRING_RELOCATE, code, reg, ex, offset, "need to relocate string : %ld", value);
    //        return false;
    //    }
    //    value = it.call(vm, IRQ_STR_RELOCATE, code, reg, ex, offset, (void*)value);
    //    if (value == 0)
    //    {
    //        VM_ERROR(ERR_STRING_RELOCATE, code, reg, ex, offset, "relocate string : %ld ERROR", value);
    //        return false;
    //    }
    //}
    //return r.write(value, ex, (ex != r.type));
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

inline int vm_cmp_type(struct regvm* vm, int v, bool i_v)
{
    return (i_v == true) ? (int)TYPE_SIGNED : vm->reg.id(v).type;
}

template <typename T> inline T vm_cmp_value(struct regvm* vm, int v, bool i_v)
{
    return (i_v == true) ? (T)v: (T)vm->reg.id(v);
}

template <typename T> inline T vm_cmp_val(const int c, const int i, const int* ms, const int* vs, const reg::v** rs)
{
    return ((c & ms[i]) != 0) ? (T)vs[i] : (T)*rs[i]; 
}

inline int vm_jcmp(struct regvm* vm, int a, int b, int c, const void* extra)
{
    code_t* p = (code_t*)extra;
    if (unlikely(p->id != CODE_DATA))
    {
        return 0;
    }
    const int next = 2;
    const int dest = p->a3;

    const int ms[] = {0x80, 0x40};
    const int vs[] = {a, b};
    const reg::v* rs[] = {&vm->reg.id(a), &vm->reg.id(b)};
    const int t1 = ((c & 0x80) != 0) ? (int)TYPE_SIGNED : rs[0]->type;
    const int t2 = ((c & 0x40) != 0) ? (int)TYPE_SIGNED : rs[1]->type;
    const int t = (t1 > t2) ? t1 : t2;

//#define CMP_VAL(type, idx) (((c & m##idx) != 0) ? (type)v##idx : (type)r##idx)
#define CMP_VAL(type, idx) vm_cmp_val<type>(c, idx, ms, vs, rs)
#define CMP_JUMP(type, cmp) return (CMP_VAL(type, 0) cmp CMP_VAL(type, 1)) ? dest : next;
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

inline int vm_calc(struct regvm* vm, int a, int b, int c)
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

frame::frame(frame& cur, func* f, code_t c, int o) :
    depth(cur.depth + 1), running(f), id(gen_id()), vm(cur.vm)
{
    caller.code = c;
    caller.offset = o;

    if (unlikely(depth <= cur.depth))
    {
        VM_ERROR(ERR_FUNCTION_CALL, c, o, "stack is OVERFLOWED !!! : %d", cur.depth);
        valid = false;
    }

    up = vm->call_stack;
    down = NULL;
    up->down = this;

    vm->call_stack = this;

    if ((unlikely(valid == false)) || (unlikely(vm->vm_call(c, o, id) == false)))
    {
        VM_ERROR(ERR_FUNCTION_CALL, c, o, "Can not get function info : %lu", id);
        valid = false;
    }
}

frame::frame(regvm* v, func* f, code_t c, int o) :
    depth(0), running(f), id(gen_id()), vm(v)
{
    caller.code = c;
    caller.offset = o;
    up = NULL;
    down = NULL;
    vm->call_stack = this;
    if (unlikely(vm->vm_call(c, o, id) == false))
    {
        VM_ERROR(ERR_FUNCTION_CALL, c, o, "Can not get function info : %lu", id);
        valid = false;
    }
}

frame::~frame()
{
    if (unlikely(vm->vm_call(caller.code, caller.offset, -id) == false))
    {
        VM_ERROR(ERR_FUNCTION_CALL, caller.code, caller.offset, "Can not get function info : %lu", id);
        valid = false;
    }

    if (up != NULL)
    {
        up->down = NULL;
    }
    vm->call_stack = up;
}

int64_t frame::gen_id(void)
{
    int64_t c = running->id;
    c <<= 32;
    c += depth;
    return c;
}

int frame::run(void)
{
    int32_t offset = 0;
    int rest = running->count;
    LOGT("running : %d - %d", offset, rest);
    const code_t* cur = running->codes + offset;
    reason = END;
    while (rest > 0)
    {
        int next = step(vm, *cur, offset, rest, cur + 1);
        if (unlikely(next == 0))
        {
            if (unlikely(reason == END))
            {
                reason = ERROR;
                VM_ERROR(ERR_RUNTIME, *cur, offset, "run code ERROR at %lld : %s - %d - %d - %d", offset, CODE_NAME(cur->id), cur->a, cur->b, cur->c);
            }
            return reason;
        }

        cur += next;
        rest -= next;
        offset += next;
    }

    if (unlikely(rest < 0))
    {
        VM_ERROR(ERR_RUNTIME, code_t{0}, offset, "run to UNEXPECT POSISTION : %d - %lld", rest, offset);
        vm->fatal = true;
        return ERROR;
    }

    auto& r = vm->reg.id(0);
    vm->exit_code = (r.type == TYPE_NULL) ? 0 : r.value.sint;
    return reason;
}

bool frame::one_step(struct regvm* vm, const code_t code, int max, int* next, const void* extra)
{
    //return step(vm, code, 0, max, extra);
    return false;
}


inline int frame::step(struct regvm* vm, code_t inst, int offset, int max, const void* extra)
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

#define JUMPS_CMP(t, cmp) vm_cmp_value<t>(vm, inst.a, i_a) cmp vm_cmp_value<t>(vm, inst.b, i_b)
#define JUMPS(i, cmp)                                                       \
    case CODE_##i:                                                          \
        {                                                                   \
            const bool i_a = ((inst.c & 0x02) != 0);                        \
            const bool i_b = ((inst.c & 0x01) != 0);                        \
            int t1 = vm_cmp_type(vm, inst.a, i_a);                          \
            int t2 = vm_cmp_type(vm, inst.b, i_b);                          \
            int dest = ((code_t*)extra)->a3;                                \
            switch ((t1 > t2) ? t1 : t2)                                    \
            {                                                               \
            case TYPE_SIGNED:                                               \
                return (JUMPS_CMP(int64_t, cmp)) ? dest : 2;                \
            case TYPE_UNSIGNED:                                             \
                return (JUMPS_CMP(uint64_t, cmp)) ? dest : 2;               \
            case TYPE_DOUBLE:                                               \
                return (JUMPS_CMP(double, cmp)) ? dest : 2;                 \
            default:                                                        \
                UNSUPPORT_TYPE(#i, (t1 > t2) ? t1 : t2, inst, offset);      \
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

    case CODE_SET:
        return vm_set(vm, inst.a, inst.b, inst.c, extra);
    case CODE_JUMP:
        return inst.a3;
    case CODE_JCMP:
        return vm_jcmp(vm, inst.a, inst.b, inst.c, extra);
    case CODE_CALC:
        return vm_calc(vm, inst.a, inst.b, inst.c);
    case CODE_EXIT:
        //vm->exit_code = ((unsigned int)inst.a2 != 255) ? inst.b2 : (int64_t)vm->reg.id(inst.a2);
        vm->exit_code = inst.b2;
        vm->exit = true;
        reason = EXIT;
        return 0;
    case CODE_RET:
        reason = RET;
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
        fprintf(stderr, "code %d is NOT SUPPORT YET\n", code);
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

#undef UNSUPPORT_TYPE

