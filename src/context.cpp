#include "context.h"

#include <stdlib.h>
#include <string.h>

#include "var.h"

context::context()
{
    scopes.emplace_back();
}

context::~context()
{
    if (scopes.size() != 1)
    {
        //WARNING !!!
    }
    scopes.clear();
}

void context::enter_block()
{
    scopes.emplace_back();
}

void context::leave_block()
{
    scopes.pop_back();
}

var* context::add(const code* inst)
{
    auto v = var::create(inst->type, inst->value.str);
    scopes.back().add(v);
    return v;
}

var* context::get(const char* name)
{
    const int l = strlen(name);
    uint32_t h = var::calc_hash(name, l);
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
        auto v = it->get(h, name, l);
        if (v != NULL)
        {
            return v;
        }
    }
    return NULL;
}

