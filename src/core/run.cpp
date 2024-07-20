#include <regvm.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "ext.h"
#include "reg.h"
#include "func.h"

//#define UNSUPPORT_TYPE(op, t, c, o) VM_ERROR(ERR_TYPE_MISMATCH, c, o, "UNSUPPORT %s value type : %d", op, t); 
#define UNSUPPORT_TYPE(op, t, c, o)

using namespace core;

bool vm_conv_impl(struct regvm* vm, reg::v& r, int to);

extern "C"
{

bool regvm_exec(struct regvm* vm, const code_t* start, int count, int64_t* exit)
{
    if (vm->run(start, count) == false)
    {
        //TODO : ERROR
        //printf("\e[31m run ERROR at %d\e[0m\n", count - rest);
        return false;
    }
    *exit = vm->exit_code;
    return true;
}

int regvm_exec_step(struct regvm* vm, const code_t* code, int max)
{
    int next = 0;
    return (core::func::step(vm, code, 0, max, &next) == false) ? 0 : next;
}

int regvm_code_len(code_t code)
{
    int count = 1;
    switch (code.id)
    {
    case CODE_NOP:
        count += code.ex;
        break;
    case CODE_EXIT:
        break;
    default:
        if (code.id >= 224)
        {
            count += 4;
        }
        else if (code.id >= 192)
        {
            count += 2;
        }
        else if (code.id >= 128)
        {
            count += 1;
        }
        break;
    }
    return count;
}

}   //extern C

int vm_CODE_NOP(regvm* vm, int reg, int ex, int offset)
{
    return ex + 1;
}

int vm_CODE_TRAP(regvm* vm, int reg, int ex, int offset)
{
    //return vm->idt.call(vm, IRQ_TRAP, *code, offset, &vm->call_stack->running->src, *next);
    return 1;
}


int vm_CODE_MOVE(regvm* vm, int reg, int ex, int offset)
{
    auto& e = vm->reg.id(ex);
    auto& r = vm->reg.id(reg);
    r.write(e.value.uint, e.type, true);
    return 1;
}


int vm_CODE_CLEAR(regvm* vm, int reg, int ex, int offset)
{
    auto& r = vm->reg.id(reg);
    if (r.write(0, ex, true) == false)
    {
        return 0;
    }
    switch (ex)
    {
    case TYPE_LIST:
        r.value.list_v = new uvalue::list_t();
        r.need_free = true;
        break;
    case TYPE_DICT:
        r.value.dict_v = new uvalue::dict_t();
        r.need_free = true;
        break;
    default:
        break;
    }
    return 1;
}


int vm_CODE_LOAD(regvm* vm, int reg, int ex, int offset)
{
    CRTP_CALL(vm_load, CODE_LOAD, reg, ex, offset, -1);
    return 1;
}


int vm_CODE_STORE(regvm* vm, int reg, int ex, int offset)
{
    CRTP_CALL(vm_store, CODE_STORE, reg, ex, offset, -1);
    return 1;
}


int vm_CODE_GLOBAL(regvm* vm, int reg, int ex, int offset)
{
    CRTP_CALL(vm_store, CODE_GLOBAL, reg, ex, offset, -1);
    return 1;
}


int vm_CODE_NEW(regvm* vm, int reg, int ex, int offset)
{
    CRTP_CALL(vm_new, CODE_NEW, reg, ex, offset, -1);
    return 1;
}


int vm_CODE_BLOCK(regvm* vm, int reg, int ex, int offset)
{
    CRTP_CALL(vm_block, CODE_BLOCK, reg, ex, offset, -1);
    return 1;
}


int vm_CODE_CALL(regvm* vm, int reg, int ex, int offset)
{
    auto& r = vm->reg.id(reg);
    if ((vm->call(r, CODE_CALL, reg, ex, offset) == false) || (vm->fatal == true))
    {
        return 0;
    }
    if (vm->exit == true)
    {
        return 0;
    }
    return 1;
}


int vm_CODE_RET(regvm* vm, int reg, int ex, int offset)
{
    return 0;
}


#define FROM_REG vm->reg.id(ex)
#define DIRECT ex
#define CALC(name, op, val)                                 \
int vm_##name(regvm* vm, int reg, int ex, int offset)       \
{                                                           \
    auto& r = vm->reg.id(reg);                              \
    switch (r.type)                                         \
    {                                                       \
    case TYPE_SIGNED:                                       \
        r.value.sint op (int64_t)val;                       \
        return 1;                                           \
    case TYPE_UNSIGNED:                                     \
        r.value.uint op (uint64_t)val;                      \
        return 1;                                           \
    case TYPE_DOUBLE:                                       \
        r.value.dbl op (double)val;                         \
        return 1;                                           \
    default:                                                \
        UNSUPPORT_TYPE("calc", r.type, code, offset);       \
        return 0;                                           \
    }                                                       \
}
        CALC(CODE_INC, +=, DIRECT);
        CALC(CODE_DEC, -=, DIRECT);
        CALC(CODE_ADD, +=, FROM_REG);
        CALC(CODE_SUB, -=, FROM_REG);
        CALC(CODE_MUL, *=, FROM_REG);
        CALC(CODE_DIV, /=, FROM_REG);
#undef CALC

int vm_CODE_MOD(regvm* vm, int reg, int ex, int offset)
{
    auto& r = vm->reg.id(reg);
    switch (r.type)
    {
    case TYPE_SIGNED:
        r.value.sint %= (int64_t)vm->reg.id(ex);
        return 1;
    case TYPE_UNSIGNED:
        r.value.uint %= (uint64_t)vm->reg.id(ex);
        return 1;
    default:
        UNSUPPORT_TYPE("calc", r.type, code, offset);
        return 0;
    }
}

int vm_CODE_AND(regvm* vm, int reg, int ex, int offset)
{
    return 1;
}


int vm_CODE_OR(regvm* vm, int reg, int ex, int offset)
{
    return 1;
}


int vm_CODE_XOR(regvm* vm, int reg, int ex, int offset)
{
    return 1;
}


int vm_CODE_SHL(regvm* vm, int reg, int ex, int offset)
{
    return 1;
}


int vm_CODE_SHR(regvm* vm, int reg, int ex, int offset)
{
    return 1;
}


int vm_CODE_CONV(regvm* vm, int reg, int ex, int offset)
{
    auto& r = vm->reg.id(reg);
    if (vm_conv_impl(vm, r, ex) == false)
    {
        UNSUPPORT_TYPE("conv", ex, CODE_CONV, offset);
        return 0;
    }
    return 1;
}


int vm_CODE_TYPE(regvm* vm, int reg, int ex, int offset)
{
    auto& r = vm->reg.id(reg);
    auto& e = vm->reg.id(ex);
    r.write(e.type, TYPE_UNSIGNED, true);
    return 1;
}


int vm_CODE_CHG(regvm* vm, int reg, int ex, int offset)
{
    auto& r = vm->reg.id(reg);
    switch (ex)
    {
    case 0: //clear
        r.value.uint = 0;
        return 1;
    case 1: //minus
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
            return 0;
        }
        return 1;
    case 2: //reciprocal
        switch (r.type)
        {
        case TYPE_UNSIGNED:
        case TYPE_SIGNED:
            if (vm_conv_impl(vm, r, TYPE_DOUBLE) == false)
            {
                UNSUPPORT_TYPE("chg", r.type, code, offset);
                return 0;
            }
            [[fallthrough]];
        case TYPE_DOUBLE:
            r.value.dbl = 1 / r.value.dbl;
            break;
        default:
            UNSUPPORT_TYPE("chg", r.type, code, offset);
            return 0;
        }
        return 1;
    case 3: //NOT
        if (r.type == TYPE_UNSIGNED)
        {
            r.value.uint = ~r.value.uint;
            return 1;
        }
        else
        {
            UNSUPPORT_TYPE("chg", r.type, code, offset);
            return 0;
        }
    case 4: //malloc
        if (r.type == TYPE_STRING)
        {
            //r.value.uint = ~r.value.uint;
            if (r.need_free == false)
            {
                r.set_from(NULL);
                char* p = strdup(r.value.str);
                r.value.str = p;
                r.need_free = true;
            }
            return 1;
        }
        else
        {
            UNSUPPORT_TYPE("chg", r.type, code, offset);
            return 0;
        }
    default:
        UNSUPPORT_TYPE("chg", code.ex, code, offset);
        return 0;
    }
    return 1;
}


int vm_CODE_CMP(regvm* vm, int reg, int ex, int offset)
{
    auto& r = vm->reg.id(reg);
    r.set_from(NULL);
    switch (ex)
    {
#define CMP(k, cmp)                             \
    case k:                                     \
        vm_conv_impl(vm, r, TYPE_SIGNED);       \
        r.value.sint = (r.value.sint cmp 0);    \
        return true;
        CMP(0, ==);
        CMP(1, !=);
        CMP(2, >);
        CMP(3, >=);
        CMP(4, <);
        CMP(5, <=);
#undef CMP
    default:
        UNSUPPORT_TYPE("cmp", code.ex, code, offset);
        return 0;
    }
    return 1;
}


int vm_CODE_JUMP(regvm* vm, int reg, int ex, int offset)
{
    auto& e = vm->reg.id(ex);
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


int vm_CODE_JZ(regvm* vm, int reg, int ex, int offset)
{
    auto& r = vm->reg.id(reg);
    return ((int64_t)r == 0) ? vm_CODE_JUMP(vm, reg, ex, offset) : 1;
}


int vm_CODE_JNZ(regvm* vm, int reg, int ex, int offset)
{
    auto& r = vm->reg.id(reg);
    return ((int64_t)r != 0) ? vm_CODE_JUMP(vm, reg, ex, offset) : 1;
}




bool vm_set(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    auto& r = vm->reg.id(code.reg);
    if ((code.ex == TYPE_STRING) && (value & 0x01))
    {
        //auto it = vm->strs.find(value);
        //if (it == vm->strs.end())
        //{
        //    VM_ERROR(ERR_STRING_RELOCATE, code, offset, "need to relocate string : %ld", value);
        //    return false;
        //}
        //value = (intptr_t)it->second;
        auto& it = vm->idt.isrs[IRQ_STR_RELOCATE];
        if (it.func == NULL)
        {
            VM_ERROR(ERR_STRING_RELOCATE, code, offset, "need to relocate string : %ld", value);
            return false;
        }
        value = it.call(vm, IRQ_STR_RELOCATE, code, offset, (void*)value);
        if (value == 0)
        {
            VM_ERROR(ERR_STRING_RELOCATE, code, offset, "relocate string : %ld ERROR", value);
            return false;
        }
    }
    return r.write(value, code.ex, (code.ex != r.type));
}

bool vm_conv_impl(struct regvm* vm, reg::v& r, int to)
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

void vm_cmd_echo_var(const reg::v& v)
{
    switch (v.type)
    {
    case TYPE_SIGNED:
        printf("%lld", (long long)v.value.sint);
        break;
    case TYPE_UNSIGNED:
        printf("%llu", (unsigned long long)v.value.uint);
        break;
    case TYPE_STRING:
        printf("%s", v.value.str);
        break;
    case TYPE_DOUBLE:
        printf("%f", v.value.dbl);
        break;
    default:
        break;
    }
}

int vm_cmd_echo(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    switch (v.idx)
    {
    case 3:
        vm_cmd_echo_var(vm->reg.id(args.a2));
        printf("\t");
        vm_cmd_echo_var(vm->reg.id(args.a3));
        printf("\t");
        vm_cmd_echo_var(vm->reg.id(args.a4));
        break;
    case 2:
        vm_cmd_echo_var(vm->reg.id(args.a2));
        printf("\t");
        vm_cmd_echo_var(vm->reg.id(args.a3));
        break;
    case 1:
        vm_cmd_echo_var(vm->reg.id(args.a2));
        break;
    case 0:
        break;
    default:
        return __LINE__;
    }
    printf("\n");
    r.write(v.idx, TYPE_SIGNED, true);
    return 0;
}

int vm_str_len(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    int len = strlen(v.value.str);
    r.write(len, TYPE_SIGNED, true);
    return 0;
}

int vm_str_substr(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    auto& start = vm->reg.id(args.a2);
    auto& len = vm->reg.id(args.a3);

    const int l = (int64_t)len;
    char* p = (char*)malloc(l + 1);
    memcpy(p, v.value.str + (int64_t)start, l);
    p[l] = '\0';

    r.clear();

    r.need_free = true;
    r.value.str = p;
    r.type = TYPE_STRING;

    return 0;
}

int vm_list_len(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    r.write(v.value.list_v->size(), TYPE_SIGNED, true);
    return 0;
}

int vm_list_at(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    auto idx = (uint64_t)vm->reg.id(args.a2);
    if (idx >= v.value.list_v->size())
    {
        r.write(0, TYPE_NULL, true);
    }
    else
    {
        r.load(v.value.list_v->at(idx)->crtp<REGVM_IMPL>());
    }
    return 0;
}

int vm_list_push(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    switch (args.a2)
    {
    case 0:
        v.value.list_v->push_back(CRTP_CALL(vm_var, args.a3));
        break;
    case 1:
        v.value.list_v->push_front(CRTP_CALL(vm_var, args.a3));
        break;
    default:
        return __LINE__;
    }
    r.write(1, TYPE_SIGNED, true);
    return 0;
}

int vm_list_pop(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    if (v.value.list_v->empty() == true)
    {
        r.write(0, TYPE_NULL, true);
        return 0;
    }

    switch (args.a2)
    {
    case 0:
        r.load(v.value.list_v->back()->crtp<REGVM_IMPL>());
        v.value.list_v->back()->crtp<REGVM_IMPL>()->release();
        v.value.list_v->pop_back();
        break;
    case 1:
        r.load(v.value.list_v->front()->crtp<REGVM_IMPL>());
        v.value.list_v->front()->crtp<REGVM_IMPL>()->release();
        v.value.list_v->pop_front();
        break;
    default:
        return __LINE__;
    }
    return 0;
}

int vm_list_insert(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    auto idx = (uint64_t)vm->reg.id(args.a2);
    if (idx >= v.value.list_v->size())
    {
        r.write(0, TYPE_SIGNED, true);
    }
    else
    {
        v.value.list_v->emplace(v.value.list_v->begin() + idx, CRTP_CALL(vm_var, args.a3));
        r.write(1, TYPE_SIGNED, true);
    }
    return 0;
}

int vm_list_erase(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    auto idx = (uint64_t)vm->reg.id(args.a2);
    if (idx >= v.value.list_v->size())
    {
        r.write(0, TYPE_SIGNED, true);
    }
    else
    {
        r.write(1, TYPE_SIGNED, true);
        auto it = v.value.list_v->begin() + idx;
        (*it)->crtp<REGVM_IMPL>()->release();
        v.value.list_v->erase(it);
    }
    return 0;
}

int vm_list_set(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    auto idx = (uint64_t)vm->reg.id(args.a2);
    bool result = false;
    if (idx < v.value.list_v->size())
    {
        result = v.value.list_v->at(idx)->crtp<REGVM_IMPL>()->set_val(vm->reg.id(args.a3));
    }
    r.write((result == true) ? 1 : 0, TYPE_SIGNED, true);
    return 0;
}

#define CHECK_DICT(var)                     \
    auto var = vm->reg.id(args.a2);         \
    if (var.type != TYPE_STRING)            \
    {                                       \
        return __LINE__;                    \
    }

int vm_dict_len(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    r.write(v.value.dict_v->size(), TYPE_SIGNED, true);
    return 0;
}

int vm_dict_set(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    CHECK_DICT(k);

    auto it = v.value.dict_v->find(k.value.str);
    if (it == v.value.dict_v->end())
    {
        v.value.dict_v->emplace(k.value.str, CRTP_CALL(vm_var, args.a3));
    }
    else
    {
        it->second->crtp<REGVM_IMPL>()->set_val(vm->reg.id(args.a3));
    }
    r.write(0, TYPE_SIGNED, true);
    return 0;
}

int vm_dict_get(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    CHECK_DICT(k);

    auto it = v.value.dict_v->find(k.value.str);
    if (it == v.value.dict_v->end())
    {
        r.write(0, TYPE_NULL, true);
    }
    else
    {
        r.load(it->second->crtp<REGVM_IMPL>());
    }
    return 0;
}

int vm_dict_del(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    CHECK_DICT(k);

    auto it = v.value.dict_v->find(k.value.str);
    bool rr = false;
    if (it != v.value.dict_v->end())
    {
        it->second->crtp<REGVM_IMPL>()->release();
        v.value.dict_v->erase(it);
        rr = true;
    }

    r.write(rr, TYPE_SIGNED, true);

    return 0;
}

int vm_dict_has(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    CHECK_DICT(k);

    auto it = v.value.dict_v->find(k.value.str);
    r.write((it != v.value.dict_v->end()) ? 1 : 0, TYPE_SIGNED, true);
    return 0;
}

int vm_dict_items(regvm* vm, reg::v& r, reg::v& v, const extend_args& args)
{
    r.write(v.value.dict_v->size(), TYPE_SIGNED, true);

    if (v.value.dict_v->empty() == true)
    {
        return 0;
    }

    core::uvalue::list_t* keys = NULL;
    core::uvalue::list_t* values = NULL;

#define INIT_LIST(arg, var)                             \
    {                                                   \
        auto& rr = vm->reg.id(arg);                     \
        rr.clear();                                     \
        rr.need_free = true;                            \
        rr.type = TYPE_LIST;                            \
        var = new core::uvalue::list_t();               \
        rr.value.list_v = var;                          \
    }

    switch (args.a2)
    {
    case 0:
        INIT_LIST(args.a3, keys);
        break;
    case 1:
        INIT_LIST(args.a4, values);
        break;
    case 2:
        INIT_LIST(args.a3, keys);
        INIT_LIST(args.a4, values);
        break;
    default:
        return __LINE__;
    }

#undef INIT_LIST

    for (auto& it : *v.value.dict_v)
    {
        if (keys != NULL)
        {
            auto k = CRTP_CALL(vm_var, TYPE_STRING, "");
            *k = it.first.c_str();
            keys->push_back(k);
        }
        if (values != NULL)
        {
            it.second->acquire();
            values->push_back(it.second);
        }
    }

    return 0;
}

#undef CHECK_DICT

#undef UNSUPPORT_TYPE

vm_op_t  vm_ops[128]    =
{
    vm_CODE_NOP,
    vm_CODE_TRAP,
    vm_CODE_MOVE,
    vm_CODE_CLEAR,
    vm_CODE_LOAD,
    vm_CODE_STORE,
    vm_CODE_GLOBAL,
    vm_CODE_NEW,
    vm_CODE_BLOCK,
    vm_CODE_CALL,
    vm_CODE_RET,
    vm_CODE_INC,
    vm_CODE_DEC,
    vm_CODE_ADD,
    vm_CODE_SUB,
    vm_CODE_MUL,
    vm_CODE_DIV,
    vm_CODE_MOD,
    vm_CODE_AND,
    vm_CODE_OR,
    vm_CODE_XOR,
    vm_CODE_SHL,
    vm_CODE_SHR,
    vm_CODE_CONV,
    vm_CODE_TYPE,
    vm_CODE_CHG,
    vm_CODE_CMP,
    vm_CODE_JUMP,
    vm_CODE_JZ,
    vm_CODE_JNZ,
};

vm_sub_op_t  cmd_ops[16]    =
{
    vm_cmd_echo,
};

vm_sub_op_t  str_ops[16]    =
{
    vm_str_len,
    vm_str_substr,
};

vm_sub_op_t  list_ops[16]   =
{
    vm_list_len,
    vm_list_at,
    vm_list_push,
    vm_list_pop,
    vm_list_insert,
    vm_list_erase,
    vm_list_set,
};

vm_sub_op_t  dict_ops[16]   =
{
    vm_dict_len,
    vm_dict_set,
    vm_dict_get,
    vm_dict_del,
    vm_dict_has,
    vm_dict_items,
};



