#include <regvm.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "ext.h"
#include "reg.h"
#include "log.h"
#include "func.h"


using namespace core;

extern bool vm_conv_type(struct regvm* vm, reg::v& r, int to);

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
    return (core::func::one_step(vm, *code, max, &next, code + 1) == false) ? 0 : next;
}

int regvm_code_len(code_t code)
{
    return 1;
}

}   //extern C


int vm_CODE_NOP(regvm* vm, code_t code, int offset, const void* extra)
{
    return 1;
}

int vm_CODE_TRAP(regvm* vm, code_t code, int offset, const void* extra)
{
    vm->idt.call(vm, IRQ_TRAP, code, offset, &vm->call_stack->running->src);
    return 1;
}

int vm_CODE_CLEAR(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& r = vm->reg.id(code.a);
    const int ex = code.b;
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

int vm_CODE_LOAD(regvm* vm, code_t code, int offset, const void* extra)
{
    return 0;
}

int vm_CODE_STORE(regvm* vm, code_t code, int offset, const void* extra)
{
    return 0;
}

int vm_CODE_GLOBAL(regvm* vm, code_t code, int offset, const void* extra)
{
    return 0;
}

int vm_CODE_NEW(regvm* vm, code_t code, int offset, const void* extra)
{
    return 0;
}

int vm_CODE_BLOCK(regvm* vm, code_t code, int offset, const void* extra)
{
    return 0;
}

int vm_CODE_CONV(regvm* vm, code_t code, int offset, const void* extra)
{
    const auto& src = vm->reg.id(code.b);
    auto& res = vm->reg.id(code.a);
    res.copy(src);
    if (vm_conv_type(vm, res, code.c) == false)
    {
        UNSUPPORT_TYPE("conv", code.c, code, offset);
        return 0;
    }
    return 1;
}

int vm_CHG_MINUS(regvm* vm, int a, int b, int c, int offset)
{
    auto& r = vm->reg.id(a);
    if (a != b)
    {
        r.copy(vm->reg.id(b));
    }
    switch (r.type)
    {
    case TYPE_UNSIGNED:
        vm_conv_type(vm, r, TYPE_SIGNED);
        [[fallthrough]];
    case TYPE_SIGNED:
        r.value.sint = 0 - r.value.sint;
        break;
    case TYPE_DOUBLE:
        r.value.dbl = 0 - r.value.dbl;
        break;
    default:
        UNSUPPORT_TYPE("chg", r.type, code_t{b}, offset);
        return 0;
    }
    return 1;
};

int vm_CHG_RECIPROCAL(regvm* vm, int a, int b, int c, int offset)
{
    auto& r = vm->reg.id(a);
    if (a != b)
    {
        const auto& s = vm->reg.id(b);
        uvalue v;
        v.dbl = 1.0 / (double)s;
        r.write(v.uint, TYPE_DOUBLE, true);
    }
    else
    {
        vm_conv_type(vm, r, TYPE_DOUBLE);
        r.value.dbl = 1.0 / r.value.dbl;
    }
    return 1;
}

int vm_CHG_NOT(regvm* vm, int a, int b, int c, int offset)
{
    auto& s = vm->reg.id(b);
    if (s.type != TYPE_UNSIGNED)
    {
        return 0;
    }
    if (a != b)
    {
        auto& r = vm->reg.id(a);
        r.write(~s.value.uint, TYPE_UNSIGNED, true);
    }
    else
    {
        s.value.uint = ~s.value.uint;
    }
    return 1;
}

int vm_CHG_MALLOC(regvm* vm, int a, int b, int c, int offset)
{
    auto& s = vm->reg.id(b);
    if (s.type != TYPE_STRING)
    {
        return 0;
    }

    if (a == b)
    {
        if (s.need_free == true)
        {
            return 1;
        }
        //r.set_from(NULL);
        char* p = strdup(s.value.str);
        s.value.str = p;
        s.need_free = true;
    }
    else
    {
        auto& r = vm->reg.id(a);
        vm_conv_type(vm, r, TYPE_STRING);
        r.set_from(NULL);
        r.value.str = strdup(s.value.str);
        r.need_free = true;
    }
    return 1;
}

int vm_CODE_CALL(regvm* vm, code_t code, int offset, const void* extra)
{
    //auto& r = vm->reg.id(reg);
    //if ((vm->call(r, code, reg, ex, offset) == false) || (vm->fatal == true))
    //{
    //    return 0;
    //}
    //if (vm->exit == true)
    //{
    //    return 1;
    //}
    return 1;
}

int vm_CODE_RET(regvm* vm, code_t code, int offset, const void* extra)
{
    return 1;
}


int vm_CODE_SET(regvm* vm, code_t code, int offset, const void* extra)
{
    uint64_t v = code.c;
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
    auto& r = vm->reg.id(code.a);
    r.write(v, code.b, (code.b != r.type));
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

inline void vm_cmd_echo_var(const reg::v& v)
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

int vm_CODE_ECHO(regvm* vm, code_t code, int offset, const void* extra)
{
    switch (code.a)
    {
    case 2:
        vm_cmd_echo_var(vm->reg.id(code.b));
        printf("\t");
        vm_cmd_echo_var(vm->reg.id(code.c));
        break;
    case 1:
        vm_cmd_echo_var(vm->reg.id(code.b));
        break;
    case 0:
        break;
    default:
        return __LINE__;
    }
    printf("\n");
    return (code.a <= 2) ? 1 : 2;
}

#if 0
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

#endif  //if 0

vm_sub_op_t  CHG_OPS[16] =
{
    vm_CHG_MINUS,
    vm_CHG_RECIPROCAL,
    vm_CHG_NOT,
    vm_CHG_MALLOC,
};

//vm_sub_op_t  cmd_ops[16]    =
//{
//    vm_cmd_echo,
//};
//
//vm_sub_op_t  str_ops[16]    =
//{
//    vm_str_len,
//    vm_str_substr,
//};
//
//vm_sub_op_t  list_ops[16]   =
//{
//    vm_list_len,
//    vm_list_at,
//    vm_list_push,
//    vm_list_pop,
//    vm_list_insert,
//    vm_list_erase,
//    vm_list_set,
//};
//
//vm_sub_op_t  dict_ops[16]   =
//{
//    vm_dict_len,
//    vm_dict_set,
//    vm_dict_get,
//    vm_dict_del,
//    vm_dict_has,
//    vm_dict_items,
//};


