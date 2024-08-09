#include "blocks.h"

#include <log.h>

#include "common.h"

const selector::reg blocks::new_var(const std::string_view& name)
{
    auto v = sel.bind(name);
    if (v.ptr != NULL)
    {
        LOGT("add var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
        stack.front().vars.emplace(name, v);
    }
    return v;
}

bool blocks::bind_var(const std::string_view& name, const selector::reg& v)
{
    auto it = stack.front().vars.find(name);
    if (it != stack.front().vars.end())
    {
        LOGE("try to bind var %d:%d:%s to block %d, but it's already bind to %d:%d", (int)v, v.ver, VIEW(name), (int)stack.size(), (int)it->second, it->second.ver);
        return false;
    }

    if (sel.bind(name, v) == false)
    {
        LOGW("bind var %d:%d:%s to block %d ERROR", (int)v, v.ver, VIEW(name), (int)stack.size());
        return false;
    }

    LOGT("bind var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
    stack.front().vars.emplace(name, v);
    return true;
}

bool blocks::enter()
{
    stack.emplace_front(sel);
    LOGT("enter block %d", (int)stack.size());
    return true;
}

bool blocks::leave()
{
    LOGT("leave block %d", (int)stack.size());
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
