#pragma once

#include <regvm.h>
#include <irq.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "reg.h"
#include "ext.h"
#include "frame.h"

#include <log.h>


#define UNSUPPORT_TYPE(op, t, c, o, ...) VM_ERROR(ERR_TYPE_MISMATCH, c, o, "UNSUPPORT %s value type : %d", op, t); 

typedef int (*vm_sub_op_t)(regvm* vm, const int a, const int b, const int c, int offset);

extern vm_sub_op_t  CHG_OPS[16];



inline bool vm_conv(struct regvm* vm, core::reg::v& r, const int to)
{
    //if (r.type == to) return true;

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
            LOGE("Can not conv to type %d", to);
            return false;
        }
    }

    r.set_from(NULL);
    r.type = to;

    return true;
}

inline bool vm_clear(struct regvm* vm, core::reg::v& r, const int type)
{
    r.clear();

    switch (type)
    {
    case TYPE_LIST:
        r.value.list_v = new core::uvalue::list_t();
        r.need_free = true;
        r.type = type;
        return true;
    case TYPE_DICT:
        r.value.dict_v = new core::uvalue::dict_t();
        r.need_free = true;
        r.type = type;
        return true;
    case TYPE_SIGNED:
    case TYPE_UNSIGNED:
        r.type = type;
        r.value.sint = 0;
        return true;
    case TYPE_DOUBLE:
        r.type = type;
        r.value.dbl = 0.0;
        return true;
    case TYPE_NULL:
        r.type = 0;
        r.value.sint = 0;
        return true;
    default:
        return true;
    }
}

inline int vm_move(regvm* vm, const int a, const int b, const int c)
{
    auto& dst = vm->reg.id(a);
    auto& src = vm->reg.id(b);
    dst.copy(src);
    if (unlikely((c != 0) && (c != src.type)))
    {
        vm_conv(vm, dst, c);
    }
    return 1;
}

inline int vm_set(regvm* vm, const int a, const int b, const int c, const void* extra)
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

    if (b == TYPE_STRTAB)
    {
        if (unlikely(vm->str_tab == NULL))
        {
            LOGE("need to relocate string : %lld, but str tab is NULL", (long long)v);
            VM_ERROR(ERR_STRING_RELOCATE, code_t{0}, -1, "need to relocate string : %lld, but str tab is NULL", (long long)v);
            return 0;
        }
        const char* s = vm->str_tab + v;
        v = (uintptr_t)s;
    }

    auto& r = vm->reg.id(a);
    r.write(v, b, (b != r.type));
    return next;
}

inline int vm_cmp_type(struct regvm* vm, int v, bool i_v)
{
    return (i_v == true) ? (int)TYPE_SIGNED : vm->reg.id(v).type;
}

template <typename T> inline T vm_cmp_value(struct regvm* vm, int v, bool i_v)
{
    return (i_v == true) ? (T)v: (T)vm->reg.id(v);
}

template <typename T> inline T vm_cmp_val(const int c, const int i, const int* ms, const int* vs, const core::reg::v** rs)
{
    return ((c & ms[i]) != 0) ? (T)vs[i] : (T)*rs[i]; 
}

inline int vm_jcmp(struct regvm* vm, const int a, const int b, const int c, const void* extra)
{
    code_t* p = (code_t*)extra;
    if (unlikely(p->id != CODE_DATA))
    {
        LOGE("Can not find CODE_DATA next to CODE_JCMP");
        return 0;
    }
    const int next = 2;
    const int dest = p->a3;

    const int ms[] = {0x80, 0x40};
    const int vs[] = {a, b};
    const core::reg::v* rs[] = {&vm->reg.id(a), &vm->reg.id(b)};
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

inline int vm_calc(struct regvm* vm, const int a, const int b, const int c)
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
            LOGE("type %d does NOT support op : %s", v.type, #op);      \
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
            LOGE("type %d does NOT support op : %s", v.type, #op);      \
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

