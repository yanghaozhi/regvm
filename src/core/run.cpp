#include <regvm.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "ext.h"
#include "reg.h"
#include "log.h"
#include "frame.h"


using namespace core;

extern bool vm_conv_type(struct regvm* vm, reg::v& r, int to);

extern "C"
{

bool regvm_func(struct regvm* vm, int32_t id, const code_t* start, int count, const struct regvm_src_location* src, int mode)
{
    if (id <= 0)
    {
        LOGE("function id can not be : %d", id);
        return false;
    }

    auto r = vm->funcs.try_emplace(id, start, count, id, src, mode);
    if (r.second == false)
    {
        LOGE("Can not reg function : %d", id);
        return false;
    }

    return true;
}

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
    return (core::frame::one_step(vm, *code, max, &next, code + 1) == false) ? 0 : next;
}

bool regvm_str_tab(struct regvm* vm, const char* str_tab)
{
    if (vm->str_tab != NULL)
    {
        LOGW("replace str tab from %p to %p", vm->str_tab, str_tab);
    }
    vm->str_tab = str_tab;
    return true;
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

inline bool clear_reg(reg::v& r, const int type)
{
    switch (type)
    {
    case TYPE_LIST:
        r.clear();
        r.value.list_v = new uvalue::list_t();
        r.need_free = true;
        r.type = type;
        return true;
    case TYPE_DICT:
        r.clear();
        r.value.dict_v = new uvalue::dict_t();
        r.need_free = true;
        r.type = type;
        return true;
    case TYPE_SIGNED:
    case TYPE_UNSIGNED:
        return r.write(0, type, true);
    case TYPE_DOUBLE:
        r.clear();
        r.type = type;
        r.value.dbl = 0.0;
        return true;
    default:
        return false;
    }
}

int vm_CODE_CLEAR(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& r = vm->reg.id(code.a);
    return (int)clear_reg(r, code.b);
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
    return 1;
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
    uint32_t id = code.b2;
    int next = 1;
    if (unlikely(id > 0x7FFF))
    {
        code_t* p = (code_t*)extra;
        if (unlikely(p->id != CODE_DATA))
        {
            VM_ERROR(ERR_RUNTIME, code, offset, "call function : %u, but does NOT find CODE after", id);
            return 0;
        }
        id += (((uint32_t)p->a3) << 8);
        next += 1;
    }

    if (unlikely(vm->call(id, code, offset) == false))
    {
        VM_ERROR(ERR_RUNTIME, code, offset, "call function : %d ERROR", id);
        return 0;
    }

    return next;
    return 1;
}

int vm_CODE_RET(regvm* vm, code_t code, int offset, const void* extra)
{
    return 0;
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

inline void vm_cmd_echo_vars(regvm* vm, int a = -1, int b = -1, int c = -1)
{
    if (a < 0) return;

    vm_cmd_echo_var(vm->reg.id(a));

    if (b < 0) return;
    printf("\t");

    vm_cmd_echo_var(vm->reg.id(b));

    if (c < 0) return;
    printf("\t");

    vm_cmd_echo_var(vm->reg.id(c));
}


int vm_CODE_ECHO(regvm* vm, code_t code, int offset, const void* extra)
{
    int r = 1;
    const int count = code.a;
    switch (count)
    {
    case 2:
        vm_cmd_echo_vars(vm, code.b, code.c);
        break;
    case 1:
        vm_cmd_echo_vars(vm, code.b);
        break;
    case 0:
        break;
    default:
        vm_cmd_echo_vars(vm, code.b, code.c);
        {
            const int loop = (count - 2) / 3;
            r += loop;
            code_t* p = (code_t*)extra;
            for (int i = 0; i < loop; i++)
            {
                if (p->id != CODE_DATA) return 0;
                printf("\t");
                vm_cmd_echo_vars(vm, p->a, p->b, p->c);
                p += 1;
            }
            if (p->id == CODE_DATA)
            {
                switch (count - 2 - loop * 3)
                {
                case 1:
                    printf("\t");
                    vm_cmd_echo_vars(vm, p->a);
                    r += 1;
                    break;
                case 2:
                    printf("\t");
                    vm_cmd_echo_vars(vm, p->a, p->b);
                    r += 1;
                    break;
                default:
                    break;
                }
            }
        }
        break;
    }
    printf("\n");
    return r;
}

int vm_CODE_SLEN(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& r = vm->reg.id(code.a);
    auto& s = vm->reg.id(code.b);
    int len = strlen(s.value.str);
    r.write(len, TYPE_SIGNED, true);
    return 1;
}

#if 0
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
#endif

inline int list_idx(regvm* vm, uvalue::list_t* l, int pos)
{
    const int64_t idx = (int64_t)vm->reg.id(pos);
    if (likely(idx >= 0))
    {
        return (idx < (int64_t)l->size()) ? idx : -1;
    }
    else
    {
        return l->size() + idx;
    }
}

int vm_CODE_LLEN(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& r = vm->reg.id(code.a);
    auto& l = vm->reg.id(code.b);
    r.write(l.value.list_v->size(), TYPE_SIGNED, true);
    return 1;
}

int vm_CODE_LAT(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& r = vm->reg.id(code.a);
    auto& l = vm->reg.id(code.b);
    const int idx = list_idx(vm, l.value.list_v, code.c);
    if (unlikely(idx < 0))
    {
        r.write(0, TYPE_NULL, true);
    }
    else
    {
        r.load(l.value.list_v->at(idx));
    }
    return 1;
}

int vm_CODE_LPUSH(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& l = vm->reg.id(code.a);
    switch (code.b)
    {
    case 0:
        l.value.list_v->push_back(CRTP_CALL(vm_var, code.c));
        break;
    case 1:
        l.value.list_v->push_front(CRTP_CALL(vm_var, code.c));
        break;
    default:
        return 0;
    }
    return 1;
}

int vm_CODE_LPOP(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& l = vm->reg.id(code.a);
    if (unlikely(l.value.list_v->empty() == true))
    {
        LOGW("try to pop from an empty list");
        return 1;
    }

    switch (code.b)
    {
    case 0:
        l.value.list_v->back()->release();
        l.value.list_v->pop_back();
        break;
    case 1:
        l.value.list_v->front()->release();
        l.value.list_v->pop_front();
        break;
    default:
        return 0;
    }
    return 1;
}

int vm_CODE_LINSERT(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& l = vm->reg.id(code.a);
    const int idx = list_idx(vm, l.value.list_v, code.b);
    if (likely(idx > 0))
    {
        l.value.list_v->emplace(l.value.list_v->begin() + idx, CRTP_CALL(vm_var, code.c));
        return 1;
    }
    else
    {
        return 0;
    }
}

int vm_CODE_LERASE(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& l = vm->reg.id(code.a);
    const int idx = list_idx(vm, l.value.list_v, code.b);
    if (unlikely(idx < 0))
    {
        return 0;
    }
    else
    {
        auto it = l.value.list_v->begin() + idx;
        (*it)->release();
        l.value.list_v->erase(it);
        return 1;
    }
}

int vm_CODE_LSET(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& l = vm->reg.id(code.a);
    const int idx = list_idx(vm, l.value.list_v, code.b);
    if (likely(idx >= 0))
    {
        l.value.list_v->at(idx)->set_val(vm->reg.id(code.c));
        return 1;
    }
    else
    {
        return 0;
    }
}

#define CHECK_DICT(var, arg)                \
    auto var = vm->reg.id(code.arg);        \
    if (var.type != TYPE_STRING)            \
    {                                       \
        return 0;                           \
    }

int vm_CODE_DLEN(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& r = vm->reg.id(code.a);
    auto& v = vm->reg.id(code.b);
    r.write(v.value.dict_v->size(), TYPE_SIGNED, true);
    return 1;
}

int vm_CODE_DSET(regvm* vm, code_t code, int offset, const void* extra)
{
    CHECK_DICT(k, b);
    auto& v = vm->reg.id(code.a);
    auto it = v.value.dict_v->find(k.value.str);
    if (it == v.value.dict_v->end())
    {
        v.value.dict_v->emplace(k.value.str, CRTP_CALL(vm_var, code.c));
    }
    else
    {
        it->second->set_val(vm->reg.id(code.c));
    }
    return 1;
}

int vm_CODE_DGET(regvm* vm, code_t code, int offset, const void* extra)
{
    CHECK_DICT(k, c);

    auto& r = vm->reg.id(code.a);
    auto& v = vm->reg.id(code.b);
    auto it = v.value.dict_v->find(k.value.str);
    if (unlikely(it == v.value.dict_v->end()))
    {
        r.write(0, TYPE_NULL, true);
    }
    else
    {
        r.load(it->second);
    }
    return 1;
}

int vm_CODE_DDEL(regvm* vm, code_t code, int offset, const void* extra)
{
    CHECK_DICT(k, b);

    auto& v = vm->reg.id(code.a);
    auto it = v.value.dict_v->find(k.value.str);
    if (likely(it != v.value.dict_v->end()))
    {
        it->second->release();
        v.value.dict_v->erase(it);
    }

    return 1;
}

int vm_CODE_DHAS(regvm* vm, code_t code, int offset, const void* extra)
{
    CHECK_DICT(k, c);

    auto& r = vm->reg.id(code.a);
    auto& v = vm->reg.id(code.b);
    auto it = v.value.dict_v->find(k.value.str);
    r.write((it != v.value.dict_v->end()) ? 1 : 0, TYPE_SIGNED, true);
    return 1;
}

int vm_CODE_DITEMS(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& v = vm->reg.id(code.a);
    auto& ks = vm->reg.id(code.b);
    auto& vs = vm->reg.id(code.c);

    clear_reg(ks, TYPE_LIST);
    clear_reg(vs, TYPE_LIST);

    if (v.value.dict_v->empty() == true)
    {
        return 1;
    }

    //core::uvalue::list_t* keys = ks.value.list_v;
    core::uvalue::list_t* values = vs.value.list_v;

    for (auto& it : *v.value.dict_v)
    {
        //TODO
        //auto k = CRTP_CALL(vm_var, TYPE_STRING, "");
        //*k = it.first.c_str();
        //keys->push_back(k);

        it.second->acquire();
        values->push_back(it.second);
    }

    return 1;
}

#undef CHECK_DICT

vm_sub_op_t  CHG_OPS[16] =
{
    vm_CHG_MINUS,
    vm_CHG_RECIPROCAL,
    vm_CHG_NOT,
    vm_CHG_MALLOC,
};



