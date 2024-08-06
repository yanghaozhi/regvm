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

const selector::reg blocks::find_var(const std::string_view& name)
{
    for (auto& it : stack)
    {
        auto v = it.vars.find(name);
        if (v != it.vars.end())
        {
            if (sel.active(v->second) == false)
            {
                //TODO : reload
                return selector::reg();
            }
            return v->second;
        }
    }
    return selector::reg();
}

bool blocks::enter()
{
    stack.emplace_front(sel);
    return true;
}

bool blocks::leave()
{
    stack.pop_front(sel);
    return true;
}

blocks::block::~block()
{
    for (auto& it : vars)
    {
        sel.release(it.second);
    }
}
