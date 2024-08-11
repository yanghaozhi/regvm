#include "blocks.h"

#include <log.h>

#include "common.h"

const selector::reg blocks::new_var(const std::string_view& name, int attr)
{
    auto v = (attr == 0) ? sel.bind(name) : sel.lock();
    if (v.ptr != NULL)
    {
        LOGT("add var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
        stack.front().vars.emplace(name, var{v, attr});
    }
    return v;
}

bool blocks::bind_var(const std::string_view& name, const selector::reg& v, int attr)
{
    auto it = stack.front().vars.find(name);
    if (it != stack.front().vars.end())
    {
        LOGE("try to bind var %d:%d:%s to block %d, but it's already bind to %d:%d", (int)v, v.ver, VIEW(name), (int)stack.size(), (int)it->second.r, it->second.r.ver);
        return false;
    }

    if (attr == 0)
    {
        if (sel.bind(name, v) == false)
        {
            LOGW("bind var %d:%d:%s to block %d ERROR", (int)v, v.ver, VIEW(name), (int)stack.size());
            return false;
        }
        LOGT("bind var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
        stack.front().vars.emplace(name, var{v, attr});
    }
    else
    {
        if (sel.lock(v) == false)
        {
            LOGW("lock for var %d:%d:%s to block %d ERROR", (int)v, v.ver, VIEW(name), (int)stack.size());
            return false;
        }
        LOGT("lock var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
        stack.front().vars.emplace(name, var{v, attr});
    }

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
        sel.release(it.second.r);
    }
}
