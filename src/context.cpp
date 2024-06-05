#include "context.h"

#include <stdlib.h>
#include <string.h>

#include "var.h"

context::context(scope& g, context* cur) : globals(g)
{
    scopes.emplace_back();

    up = cur;
    down = NULL;
    if (cur != NULL)
    {
        cur->down = this;
    }
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

var* context::add(const code* inst, uint8_t type)
{
    auto v = var::create(type, inst->value.str);
    scopes.back().add(v);
    return (v->release() == true) ? v : NULL;
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
    return globals.get(h, name, l);
}

