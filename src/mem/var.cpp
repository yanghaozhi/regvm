#include "var.h"

#include "../core/reg.h"

#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

using namespace ext;

var::var(uint8_t t, const char* n, const int l) :
    type(t),
    name_len(l),
    hash(calc_hash(n, l))
{
    strcpy(name, n);
    value.uint = 0;
}

var::~var()
{
    if (reg != NULL)
    {
        reg->set_from(NULL);
    }
    free_uvalue(type, value);
}

var* var::create(uint8_t t, const char* n)
{
    const int l = strlen(n);

    char* p = (char*)malloc(sizeof(var) + l + 1);
    var* v = new (p) var(t, n, l);

    return v;
}

bool var::cmp(uint32_t k, const char* n, int l)
{
    if ((k != hash) || (l != name_len))
    {
        return false;
    }
    return (memcmp(name, n, l)  == 0) ? true : false;
}


uint32_t var::calc_hash(const char* name, const int len)
{
    const int seed = 35153;
    uint32_t hash = 0;
    for (int i = 0; i < len; i++)
    {
        hash = hash * seed + name[i];
    }
    return hash;
}

bool var::set_val(const core::regv<var>& reg)
{
    if (type != reg.type)
    {
        return false;
    }

    switch (type)
    {
#define AGGREGATE(T, V, CP, ...)                    \
    case T:                                         \
        if (value.V != reg.value.V)                 \
        {                                           \
            if (value.V != NULL)                    \
            {                                       \
                core::free_uvalue(type, value);     \
            }                                       \
            if (reg.need_free == true)              \
            {                                       \
                value.V = reg.value.V;              \
                reg.need_free = false;              \
            }                                       \
            else                                    \
            {                                       \
                value.V = CP(__VA_ARGS__);          \
            }                                       \
        }                                           \
        break;

        AGGREGATE(TYPE_STRING, str, strdup, reg.value.str);
        AGGREGATE(TYPE_LIST, list_v, new core::uvalue::list_t, reg.value.list_v->begin(), reg.value.list_v->end());
        AGGREGATE(TYPE_DICT, dict_v, new core::uvalue::dict_t, reg.value.dict_v->begin(), reg.value.dict_v->end());

#undef AGGREGATE
    default:
        value = reg.value;
        break;
    }
    return true;
}

bool var::set_reg(const core::regv<var>* new_reg) const
{
    if (new_reg == reg) return true;

    if (new_reg != NULL)
    {
        if (reg == NULL)
        {
            ++ref;
        }
    }
    else
    {
        if (reg != NULL)
        {
            if (release() == false)
            {
                return false;
            }
        }
    }
    reg = const_cast<decltype(reg)>(new_reg);
    return true;
}

bool var::release(void) const
{
    if (--ref > 0)
    {
        return true;
    }

    if (ref == 0)
    {
        //delete this;
        this->~var();
        free((void*)this);
    }

    return false;
}

bool var::store_from(core::regv<var>& r)
{
    if (reg == &r) return r.store();

    if (type != r.type)
    {
        return false;
    }

    if (reg != NULL)
    {
        reg->set_from(NULL);
    }

    auto old = r.from;
    if (old != NULL)
    {
        //do NOT writeback
        old->set_reg(NULL);
    }

    //v->acquire();

    r.set_from(this);

    reg = &r;
    if (r.need_free == true)
    {
        value = r.value;
        r.need_free = false;
    }
    else
    {
        set_val(r);
    }

    return true;
}

