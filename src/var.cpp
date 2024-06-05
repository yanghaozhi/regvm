#include "var.h"

#include <stdlib.h>
#include <string.h>


var* var::create(uint8_t t, const char* n)
{
    const int l = strlen(n);
    auto v = (var*)malloc(sizeof(var) + l + 1);
    v->type = t;
    v->name_len = l;
    strcpy(v->name, n);
    v->hash = calc_hash(n, l);
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
    const int seed = 131;
    uint32_t hash = 0;
    for (int i = 0; i < len; i++)
    {
        hash = hash * seed + name[i];
    }
    return hash;
}

void var::set_reg(const int id)
{
    if (id == reg) return;

    if (reg == 0)
    {
        if (id != 0)
        {
            ++ref;
        }
    }
    else
    {
        if (id == 0)
        {
            release();
        }
    }
}

bool var::acquire(void)
{
    ++ref;
    return true;
}

void var::release(void)
{
    if (--ref <= 0)
    {
        //delete this;
        free(this);
    }
}

var* vars::get(const char* name)
{
    return NULL;
}
