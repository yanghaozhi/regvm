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
        //auto& r = core::reg
    }
    if (need_free == true)
    {
        switch (type)
        {
        case TYPE_STRING:
            free((void*)value.str);
            break;
        default:
            break;
        }
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

void var::set_val(int t, core::uvalue v)
{
    if (type == t)
    {
        value = v;
    }
}

void var::set_reg(core::regv<var>* new_reg)
{
    if (new_reg == reg) return;

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
            release();
        }
    }
    reg = new_reg;
}

bool var::release(void)
{
    if (--ref <= 0)
    {
        //delete this;
        this->~var();
        free(this);
        return false;
    }
    return true;
}

bool var::store(core::regv<var>& r)
{
    if (reg == &r) return r.store();

    if (type != r.type)
    {
        //TODO : error handler
        assert(0);
        return false;
    }

    core::var<var>* old = r.from;
    if (old != NULL)
    {
        //do NOT writeback
        old->set_reg(NULL);
    }

    //v->acquire();

    r.set_from(this);

    value = r.value;
    reg = &r;
    need_free = r.need_free;
    r.need_free = false;

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

