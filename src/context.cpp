#include "context.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "var.h"

context::context(scope& g, context* c, func* f) :
    globals(g)
{
    scopes.emplace_front(scopes.size() + 1);

    up = c;
    down = NULL;
    if (c != NULL)
    {
        c->down = this;
    }
}

context::~context()
{
    if (scopes.size() != 1)
    {
        assert(0);
        //WARNING !!!
    }
    scopes.clear();
}

void context::enter_block()
{
    scopes.emplace_front(scopes.size() + 1);
}

void context::leave_block()
{
    scopes.pop_front();
}

var* context::add(const int type, const char* name)
{
    auto v = var::create(type, name);
    scopes.front().add(v);
    return (v->release() == true) ? v : NULL;
}

var* context::get(const char* name) const
{
    const int l = strlen(name);
    uint32_t h = var::calc_hash(name, l);
    for (const auto& it : scopes)
    {
        auto v = it.get(h, name, l);
        if (v != NULL)
        {
            return v;
        }
    }
    return globals.get(h, name, l);
}

