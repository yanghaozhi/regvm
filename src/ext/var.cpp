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

void var::set_reg(const int id)
{
    if (id == reg) return;

    if (reg == -1)
    {
        if (id != -1)
        {
            ++ref;
        }
    }
    else
    {
        if (id == -1)
        {
            release();
        }
    }
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

bool var::store(core::regv& r)
{
    if (reg == r.idx) return r.store();

    if (type != r.type)
    {
        //TODO : error handler
        assert(0);
        return false;
    }

    core::var* old = r.from;
    if (old != NULL)
    {
        //do NOT writeback
        old->release();
    }

    //v->acquire();

    r.set_from(this);

    value = r.value;
    reg = r.idx;

    return true;
}

bool var::load(core::regv& r)
{
    if (reg == r.idx)
    {
        r.type = type;
        r.value = value;
        return true;
    }

    r.store();

    if (reg >= 0)
    {
        auto o = neighbor(&r, reg);
        o->store();
        o->set_from(NULL);
    }

    r.type = type;
    r.value = value;
    r.set_from(this);

    return true;
}

core::regv* var::neighbor(core::regv* r, int id)
{
    return r + (id - r->idx);
}

