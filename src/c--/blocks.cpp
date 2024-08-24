#include "blocks.h"

#include <log.h>

#include "common.h"
#include "statements.h"

#include "func.h"

uint32_t         blocks::var_id  = 1;



const blocks::var* blocks::bind_arg(const std::string_view& name, int r, int attr)
{
    auto v = regs.fixed(r);
    if (v.ptr == NULL)
    {
        LOGE("Can NOT bind arg %s at block %d", VIEW(name), (int)stack.size());
        return NULL;
    }
    LOGT("bind arg %d:%d:%s at block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
    return &stack.front().vars.emplace(name, var{v, attr, new_id(name)}).first->second;
}

const blocks::var* blocks::new_var(const std::string_view& name, int attr)
{
    auto v = (attr == 0) ? regs.bind(name) : regs.lock();
    if (v.ptr == NULL)
    {
        LOGE("Can NOT add var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
        return NULL;
    }
    LOGT("add var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
    auto r = stack.front().vars.emplace(name, var{v, attr, new_id(name)});
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
        if (regs.bind(name, v) == false)
        {
            LOGW("bind var %d:%d:%s to block %d ERROR", (int)v, v.ver, VIEW(name), (int)stack.size());
            return NULL;
        }
        LOGT("bind var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
        stack.front().vars.emplace(name, var{v, attr, new_id(name)});
    }
    else
    {
        if (regs.lock(v) == false)
        {
            LOGW("lock for var %d:%d:%s to block %d ERROR", (int)v, v.ver, VIEW(name), (int)stack.size());
            return NULL;
        }
        LOGT("lock var %d:%d:%s to block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
    }

    auto r = stack.front().vars.emplace(name, var{v, attr, new_id(name)});
    return &r.first->second;
}

bool blocks::enter()
{
    stack.emplace_front(this);
    LOGT("%p enter block %d", this, (int)stack.size());
    return true;
}

bool blocks::leave()
{
    LOGT("%p leave block %d", this, (int)stack.size());
    stack.pop_front();
    return true;
}

blocks::block::~block()
{
    insts_t* insts = cur->insts;
    selector& regs = cur->regs;
    int n = (vars.size() > 0) ? regs.tmp() : -1;
    for (auto& it : vars)
    {
        LOGT("deleting var %u(%u) - %s", (uint32_t)it.second.id, it.second.attr, VIEW(it.second.reg.ptr->var));

        if ((it.second.attr & REG) == 0)
        {
            INST(SET, n, TYPE_ADDR, it.second.id);
            INST(STORE, 0, n, 5);
        }

        regs.release(it.second.reg);
    }
}

uint32_t blocks::new_id(const std::string_view& name)
{
    uint32_t n = ++var_id;
    while (ids.emplace(n).second == false)
    {
        n = ++var_id;
    }
    LOGT("alloc %u for name %s", n, VIEW(name));
    return n;
}

