#include "var.h"

#include "../core/reg.h"

#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "log.h"

#define VAR     static_cast<ext::var*>(var)
#define CVAR    static_cast<const ext::var*>(var)

using namespace ext;

var::var(uint8_t t, uint64_t i) :
    id(i)
{
    typev = t;
    LOGD("create var %016llx - %d - %p", (long long)id, ref, this);
    value.uint = 0;
}

var::~var()
{
    LOGD("delete var %016llx - %d - %p", (long long)id, ref, this);
    if (reg != NULL)
    {
        reg->set_from(NULL);
    }
    free_uvalue(typev, value);
}

bool var::set_val(const core::regv& reg)
{
    if (typev != reg.type)
    {
        return false;
    }

    switch (typev)
    {
#define AGGREGATE(T, V, CP, ...)                    \
    case T:                                         \
        if (value.V != reg.value.V)                 \
        {                                           \
            if (value.V != NULL)                    \
            {                                       \
                core::free_uvalue(typev, value);    \
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

bool var::set_reg(const core::regv* new_reg) const
{
    if (new_reg == reg) return true;

    if (new_reg != NULL)
    {
        if (reg == NULL)
        {
            ++ref;
            LOGD("var %p ref : %d", this, ref);
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

bool var::store_from(core::regv& r)
{
    if (reg == &r) return r.store();

    if (typev != r.type)
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

