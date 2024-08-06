#include "blocks.h"

#include <log.h>

#include "common.h"

const selector::reg blocks::new_var(const std::string_view& name)
{
    auto v = sel.bind(name);
    if (v.ptr != NULL)
    {
        stack.front().vars.emplace(name, v);
    }
    return v;
}

bool blocks::enter()
{
    stack.emplace_front(sel);
    return true;
}

bool blocks::leave()
{
    stack.pop_front();
    return true;
}

blocks::block::~block()
{
    for (auto& it : vars)
    {
        sel.release(it.second);
    }
}
