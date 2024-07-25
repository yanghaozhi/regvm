#include <regvm.h>
#include <irq.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "ext.h"

#include <log.h>

#define UNSUPPORT_TYPE(op, t, c, o, ...) //VM_ERROR(ERR_TYPE_MISMATCH, c, o, "UNSUPPORT %s value type : %d", op, t); 

using namespace core;



extern vm_op_t      vm_ops[16];
extern vm_sub_op_t  cmd_ops[16];
extern vm_sub_op_t  str_ops[16];
extern vm_sub_op_t  list_ops[16];
extern vm_sub_op_t  dict_ops[16];


int vm_CODE_JUMP(regvm* vm, int code, int reg, int ex, int offset);

extern bool vm_set(struct regvm* vm, int code, int reg, int ex, int offset, int64_t value);

extern bool vm_str(regvm* vm, int ret, int op, reg::v& r, const extend_args& args);
extern bool vm_list(regvm* vm, int ret, int op, reg::v& r, const extend_args& args);
extern bool vm_dict(regvm* vm, int ret, int op, reg::v& r, const extend_args& args);


bool vm_extend(struct regvm* vm, int code, int reg, int ex, int offset, uint16_t* extra, int type, vm_sub_op_t* ops)
{
    extend_args args = {*extra};
    auto& v = vm->reg.id(args.a1);
    if ((type >= 0) && (v.type != type))
    {
        VM_ERROR(ERR_TYPE_MISMATCH, code, reg, ex, offset, "UNSUPPORT value type : %d - want %d", v.type, type);
        return false;
    }

    auto& r = vm->reg.id(reg);
    vm_sub_op_t f = ops[ex];
    if (f == NULL)
    {
        VM_ERROR(ERR_TYPE_MISMATCH, code, reg, ex, offset, "UNSUPPORT op type : %d of %d", ex, type);
        return false;
    }

    int errline = f(vm, r, v, args);
    if (errline > 0)
    {
        VM_ERROR(ERR_RUNTIME, code, reg, ex, offset, "run sub op ERROR : %d - %d at %d", ex, type, errline);
        return false;
    }
    return true;
}

#define NULL_CALL() true


inline bool step(struct regvm* vm, int code, int reg, int ex, int offset, int max, int* next, const void* extra)
{
    *next = 1;

#define STEP_ERROR(e, fmt, ...) VM_ERROR(e, code, reg, ex, offset, fmt, ##__VA_ARGS__);
#define CALL(f, ...)                                                    \
    if (f(__VA_ARGS__) == false)                                        \
    {                                                                   \
        STEP_ERROR(ERR_RUNTIME, "run code %u-%u-%u at %d ERROR",        \
                code, reg, ex, offset);                 \
    }

    LOGT("%d : code 0x%02X - %d - %d", offset, code, reg, ex);

    switch (code)
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
        EXTRA_RUN(NOP, ex, NULL_CALL);
        EXTRA_RUN(SETS, 1, vm_set, vm, code, reg, ex, offset, *(int16_t*)extra);
        EXTRA_RUN(SETI, 2, vm_set, vm, code, reg, ex, offset, *(int32_t*)extra);
        EXTRA_RUN(SETL, 4, vm_set, vm, code, reg, ex, offset, *(int64_t*)extra);

        EXTRA_RUN(CMD,  1, vm_extend, vm, code, reg, ex, offset, (uint16_t*)extra, -1, cmd_ops);
        EXTRA_RUN(STR,  1, vm_extend, vm, code, reg, ex, offset, (uint16_t*)extra, TYPE_STRING, str_ops);
        EXTRA_RUN(LIST, 1, vm_extend, vm, code, reg, ex, offset, (uint16_t*)extra, TYPE_LIST, list_ops);
        EXTRA_RUN(DICT, 1, vm_extend, vm, code, reg, ex, offset, (uint16_t*)extra, TYPE_DICT, dict_ops);
#undef EXTRA_RUN

#if 0
#define CODE_RUN(id, func, ...)             \
    case CODE_##id:                         \
        CALL(func, ##__VA_ARGS__);          \
        break;
        CODE_RUN(STORE, CRTP_CALL, vm_store, code, reg, ex, offset, 0);
        CODE_RUN(GLOBAL, CRTP_CALL, vm_store, code, reg, ex, offset, -1);
        CODE_RUN(NEW, CRTP_CALL, vm_new, code, reg, ex, offset, -1);
        CODE_RUN(LOAD, CRTP_CALL, vm_load, code, reg, ex, offset, -1);
        CODE_RUN(BLOCK, CRTP_CALL, vm_block, code, reg, ex, offset, vm->call_stack->id);
        CODE_RUN(MOVE, vm_move, vm, code, reg, ex);
        CODE_RUN(CLEAR, vm_clear, vm, code, reg, ex, offset);
        CODE_RUN(CONV, vm_conv, vm, code, reg, ex, offset);
        CODE_RUN(TYPE, vm_type, vm, code, reg, ex, offset);
        CODE_RUN(CHG, vm_chg, vm, code, reg, ex, offset);
        CODE_RUN(CMP, vm_cmp, vm, code, reg, ex, offset);
#undef CODE_RUN
#endif

#define FROM_REG vm->reg.id(ex)
#define DIRECT ex
#define CALC(i, op, val)                                                    \
    case CODE_##i:                                                          \
        {                                                                   \
            auto& r = vm->reg.id(reg);                                      \
            switch (r.type)                                                 \
            {                                                               \
            case TYPE_SIGNED:                                               \
                r.value.sint op (int64_t)val;                               \
                break;                                                      \
            case TYPE_UNSIGNED:                                             \
                r.value.uint op (uint64_t)val;                              \
                break;                                                      \
            case TYPE_DOUBLE:                                               \
                r.value.dbl op (double)val;                                 \
                break;                                                      \
            default:                                                        \
                UNSUPPORT_TYPE("calc", r.type, code, reg, ex, offset);      \
                break;                                                      \
            }                                                               \
        }                                                                   \
        break;
        CALC(INC, +=, DIRECT);
        CALC(DEC, -=, DIRECT);
        CALC(ADD, +=, FROM_REG);
        CALC(SUB, -=, FROM_REG);
        CALC(MUL, *=, FROM_REG);
        CALC(DIV, /=, FROM_REG);
#undef CALC

#define BITWISE(i, op)                                                      \
    case CODE_##i:                                                          \
        {                                                                   \
            auto& e = vm->reg.id(ex);                                       \
            auto& r = vm->reg.id(reg);                                      \
            if (r.type == TYPE_UNSIGNED)                                    \
            {                                                               \
                r.value.uint op (uint64_t)e;                                \
            }                                                               \
            else                                                            \
            {                                                               \
                UNSUPPORT_TYPE("bitwise", r.type, code, reg, ex, offset);   \
            }                                                               \
        }                                                                   \
        break;
        BITWISE(AND, &=);
        BITWISE(OR, |=);
        BITWISE(XOR, ^=);
#undef BITWISE

#define SHIFT(i, op)                                                        \
    case CODE_##i:                                                          \
        {                                                                   \
            auto& e = vm->reg.id(ex);                                       \
            auto& r = vm->reg.id(reg);                                      \
            switch (r.type)                                                 \
            {                                                               \
            case TYPE_SIGNED:                                               \
                r.value.sint op (uint64_t)e;                                \
                break;                                                      \
            case TYPE_UNSIGNED:                                             \
                r.value.uint op (uint64_t)e;                                \
                break;                                                      \
            default:                                                        \
                UNSUPPORT_TYPE("shift", r.type, code, reg, ex, offset);     \
                break;                                                      \
            }                                                               \
        }                                                                   \
        break;
        SHIFT(SHL, <<=);
        SHIFT(SHR, >>=);
        SHIFT(MOD, %=);
#undef BITWISE

#define JUMP(i, cmp)                                                                            \
    case CODE_##i:                                                                              \
        *next = ((int64_t)vm->reg.id(reg) cmp 0) ? vm_CODE_JUMP(vm, code, reg, ex, offset) : 1; \
        break;
        JUMP(JZ, ==);
        JUMP(JNZ, !=);
#undef JUMP

#define WRITE(i, ...)                                                       \
    case CODE_##i:                                                          \
        {                                                                   \
            auto& r = vm->reg.id(reg);                                      \
            auto& e = vm->reg.id(ex);                                       \
            return r.write(__VA_ARGS__);                                    \
        }
        WRITE(MOVE, e.value.uint, e.type, true);
        WRITE(TYPE, e.type, TYPE_UNSIGNED, true);
#undef WRITE

    case CODE_EXIT:
        vm->exit_code = (ex == 0) ? 0 : (int64_t)vm->reg.id(reg);
        vm->exit = true;
        [[fallthrough]];
    case CODE_RET:
        *next = 0;
        return true;
    default:
        if (code < (int)(sizeof(vm->ops) / sizeof(vm->ops[0])))
        {
            *next = vm->ops[code](vm, code, reg, ex, offset);
            if (*next == 0)
            {
                VM_ERROR(ERR_RUNTIME, code, reg, ex, offset, "run code ERROR : %u - %u - %u", code, reg, ex);
                return false;
            }
        }
        else
        {
            VM_ERROR(ERR_INVALID_CODE, code, reg, ex, offset, "invalid code : %u - %u - %u", code, reg, ex);
            fprintf(stderr, "code %d is NOT SUPPORT YET", code);
            return false;
        }
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
        if (step(vm, cur->id, cur->reg, cur->ex, offset, rest, &next, cur + 1) == false)
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

    if (rest < 0)
    {
        VM_ERROR(ERR_RUNTIME, 0, 0, 0, offset, "run to UNEXPECT POSISTION : %d - %lld", rest, offset);
    }

    auto& r = vm->reg.id(0);
    vm->exit_code = (r.type == TYPE_NULL) ? 0 : r.value.sint;
    return true;
}

bool func::one_step(struct regvm* vm, const code_t code, int max, int* next, const void* extra)
{
    return step(vm, code.id, code.reg, code.ex, 0, max, next, extra);
}

#undef UNSUPPORT_TYPE

