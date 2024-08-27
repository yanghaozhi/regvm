#include "blocks.h"

#include <log.h>

#include "common.h"
#include "statements.h"

#include "func.h"



const blocks::var* blocks::bind_arg(const std::string_view& name, int r, int attr)
{
    auto v = regs.fixed(r);
    if (v.ptr == NULL)
    {
        LOGE("Can NOT bind arg %s at block %d", VIEW(name), (int)stack.size());
        return NULL;
    }
    LOGT("bind arg %d:%d:%s at block %d", (int)v, v.ver, VIEW(name), (int)stack.size());
    return add_2_vars(name, v, attr);
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
    return add_2_vars(name, v, attr);
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

    return add_2_vars(name, v, attr);
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
    //int n = (vars.size() > 0) ? regs.tmp() : -1;
    for (auto& it : vars)
    {
        LOGT("deleting var %u(%u) - %s", (uint32_t)it.second.id, it.second.attr, VIEW(it.second.reg.ptr->var));

        //if ((it.second.attr & REG) == 0)
        //{
        //    INST(SET, n, TYPE_ADDR, it.second.id);
        //    INST(STORE, 0, n, 5);
        //}

        regs.release(it.second.reg);
    }
    if (last > 0)
    {
        INST(STORE, first, last + 1, 5);
    }
}

const blocks::var* blocks::add_2_vars(const std::string_view& name, const selector::reg& v, int attr)
{
    uint16_t n = var_id++;
    if (var_id < n)
    {
        LOGE("var id has warp !!!");
        return NULL;
    }

    LOGT("alloc %u for name %s", n, VIEW(name));

    auto& cur = stack.front();

    cur.last = n;
    return &cur.vars.try_emplace(name, v, attr, n).first->second;
}

