#include <regvm.h>
#include <irq.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "ext.h"
#include "run.h"
#include "frame.h"

#include <log.h>


using namespace core;



frame::frame(frame& cur, func* f, code_t c, int o) :
    depth(cur.depth + 1), running(f), id(gen_id()), vm(cur.vm)
{
    caller.code = c;
    caller.offset = o;

    if (unlikely(depth <= cur.depth))
    {
        VM_ERROR(ERR_FUNCTION_CALL, c, o, "stack is OVERFLOWED !!! : %d", cur.depth);
        valid = false;
        return;
    }

    up = vm->call_stack;
    down = NULL;
    up->down = this;

    vm->call_stack = this;

    //TODO
    //if ((unlikely(valid == false)) || (unlikely(vm->vm_call(c, o, id) == false)))
    //{
    //    VM_ERROR(ERR_FUNCTION_CALL, c, o, "Can not get function info : %lu", id);
    //    valid = false;
    //}
}

frame::frame(regvm* v, func* f, code_t c, int o) :
    depth(0), running(f), id(gen_id()), vm(v)
{
    caller.code = c;
    caller.offset = o;
    up = NULL;
    down = NULL;
    vm->call_stack = this;

    //TODO
    //if (unlikely(vm->vm_call(c, o, id) == false))
    //{
    //    VM_ERROR(ERR_FUNCTION_CALL, c, o, "Can not get function info : %lu", id);
    //    valid = false;
    //}
}

frame::~frame()
{
    //TODO
    //if (unlikely(vm->vm_call(caller.code, caller.offset, -id) == false))
    //{
    //    VM_ERROR(ERR_FUNCTION_CALL, caller.code, caller.offset, "Can not get function info : %lu", id);
    //    valid = false;
    //}

    if (up != NULL)
    {
        up->down = NULL;
    }
    vm->call_stack = up;
}

int64_t frame::gen_id(void) const
{
    int64_t c = running->id;
    c <<= 32;
    c += (depth << 16);
    return c;
}

int frame::run(void)
{
    int32_t offset = 0;
    int rest = running->count;
    LOGT("running : %lld - %d - %d @ %d - %d", (long long)id, running->id, depth, offset, rest);
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
            r.set_from(NULL);                                               \
            r.type = t;                                                     \
            switch (t)                                                      \
            {                                                               \
            case TYPE_SIGNED:                                               \
                r.value.sint = (int64_t)b op (int64_t)c;                    \
                return 1;                                                   \
            case TYPE_UNSIGNED:                                             \
                r.value.uint = (uint64_t)b op (uint64_t)c;                  \
                return 1;                                                   \
            case TYPE_DOUBLE:                                               \
                r.value.dbl = (double)b op (double)c;                       \
                return 1;                                                   \
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
            vm_conv(vm, r, TYPE_UNSIGNED);                                  \
            if (likely((b.type == TYPE_UNSIGNED) && (c.type == TYPE_UNSIGNED)))    \
            {                                                               \
                r.value.uint = (uint64_t)b op (uint64_t)c;                  \
                return 1;                                                   \
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
            vm_conv(vm, r, b.type);                                         \
            switch (r.type)                                                 \
            {                                                               \
            case TYPE_SIGNED:                                               \
                r.value.sint = (int64_t)b op (int64_t)c;                    \
                return 1;                                                   \
            case TYPE_UNSIGNED:                                             \
                r.value.uint = (uint64_t)b op (uint64_t)c;                  \
                return 1;                                                   \
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
#undef JUMPS_CMP

#define FUNC(i, func, ...)                                                  \
    case CODE_##i:                                                          \
        return func(__VA_ARGS__);
        FUNC(SET, vm_set, vm, inst.a, inst.b, inst.c, extra);
        FUNC(MOVE, vm_move, vm, inst.a, inst.b, inst.c);
        //FUNC(JCMP, vm_jcmp, vm, inst.a, inst.b, inst.c, extra);
        FUNC(CALC, vm_calc, vm, inst.a, inst.b, inst.c);
        FUNC(TYPE, vm->reg.id(inst.a).write, vm->reg.id(inst.b).type, TYPE_SIGNED, true);
        FUNC(CLEAR, vm_clear, vm, vm->reg.id(inst.a), inst.b);
#undef FUNC

    case CODE_JUMP:
        return inst.a3;
    case CODE_EXIT:
        //vm->exit_code = ((unsigned int)inst.a2 != 255) ? inst.b2 : (int64_t)vm->reg.id(inst.a2);
        vm->exit_code = inst.b2;
        vm->exit = true;
        reason = EXIT;
        return 0;
    case CODE_RET:
        reason = RET;
        return vm_code_ops[CODE_RET - CODE_TRAP](vm, inst, offset, extra);
    default:
        if (likely(code >= CODE_TRAP))
        {
            auto f = vm_code_ops[code - CODE_TRAP];
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
        return 1;
    }
    else
    {
        return 0;
    }
}

#undef UNSUPPORT_TYPE

