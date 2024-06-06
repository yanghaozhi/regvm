#include "context.h"

#include <stdlib.h>
#include <string.h>

#include "var.h"

context::context(scope& g, context* cur) :
    cur(NULL),
    func(),
    globals(g)
{
    scopes.emplace_front(scopes.size() + 1);

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
    scopes.emplace_front(scopes.size() + 1);
}

void context::leave_block()
{
    scopes.pop_front();
}

var* context::add(const code10_t* code)
{
    auto v = var::create(code->base.type, code->str);
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

