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
    switch (type)
    {
    case TYPE_STRING:
        free((void*)value.str);
        break;
    default:
        break;
    }
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

bool var::set_val(core::regv<var>* reg)
{
    return true;
}

bool var::set_val(int t, core::uvalue v)
{
    if (type != t)
    {
        return false;
    }

    switch (t)
    {
    case TYPE_STRING:
        if (value.str != NULL)
        {
            free((void*)value.str);
        }
        value.str = strdup(v.str);
        break;
    default:
        value = v;
        break;
    }
    return true;
}

bool var::set_reg(core::regv<var>* new_reg)
{
    if (new_reg == reg) return true;

    if (reg == NULL)
    {
        if (new_reg != NULL)
        {
            ++ref;
        }
    }
    else
    {
        if (new_reg == NULL)
        {
            if (release() == false)
            {
                return false;
            }
        }
    }
    reg = new_reg;
    return true;
}

bool var::release(void)
{
    if (--ref > 0)
    {
        return true;
    }

    if (ref == 0)
    {
        //delete this;
        this->~var();
        free(this);
    }

    return false;
}

bool var::store(core::regv<var>& r)
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

    core::var_type<var>* old = r.from;
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
        set_val(type, r.value);
    }

    return true;
}

bool var::load(core::regv<var>& r)
{
    if (reg == &r)
    {
        r.type = type;
        r.value = value;
        return true;
    }

    r.clear();

    if (reg != NULL)
    {
        auto o = neighbor(&r, reg->idx);
        o->clear();
    }

    r.type = type;
    r.value = value;
    r.set_from(this);
    r.need_free = false;

    return true;
}

core::regv<var>* var::neighbor(core::regv<var>* r, int id)
{
    return r + (id - r->idx);
}

