#include "blocks.h"

#include <log.h>

#include "common.h"


uint64_t         blocks::var_id  = 1;


const blocks::var* blocks::new_var(const std::string_view& name, int attr)
{
    auto v = (attr == 0) ? sel.bind(name) : sel.lock();
    if (v.ptr == NULL)
    {
        return NULL;
    }
    LOGT("add var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
    auto r = stack.front().vars.emplace(name, var{v, attr, ++var_id});
    return &r.first->second;
}

const blocks::var* blocks::bind_var(const std::string_view& name, const selector::reg& v, int attr)
{
    auto it = stack.front().vars.find(name);
    if (it != stack.front().vars.end())
    {
        LOGE("try to bind var %d:%d:%s to block %d, but it's already bind to %d:%d", (int)v, v.ver, VIEW(name), (int)stack.size(), (int)it->second.reg, it->second.reg.ver);
        return NULL;
    }

    if (attr == 0)
    {
        if (sel.bind(name, v) == false)
        {
            LOGW("bind var %d:%d:%s to block %d ERROR", (int)v, v.ver, VIEW(name), (int)stack.size());
            return NULL;
        }
        LOGT("bind var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
        stack.front().vars.emplace(name, var{v, attr, ++var_id});
    }
    else
    {
        if (sel.lock(v) == false)
        {
            LOGW("lock for var %d:%d:%s to block %d ERROR", (int)v, v.ver, VIEW(name), (int)stack.size());
            return NULL;
        }
        LOGT("lock var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
    }

    auto r = stack.front().vars.emplace(name, var{v, attr, ++var_id});
    return &r.first->second;
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
        sel.release(it.second.reg);
    }
}
