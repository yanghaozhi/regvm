#include "select.h"

#include <log.h>

#include "common.h"

class select               regs;

select::reg::reg(int i, uint32_t v) : id(i), version(v)
{
    id = regs.acquire(id, version);
}

select::reg::reg(void) : id(-1), version(0)
{
}

select::reg::reg(const reg& o) : id(regs.acquire(o.id, o.version)), version(o.version), reload(o.reload)
{
}

select::reg::reg(reg&& o) : id(o.id), version(o.version), reload(o.reload)
{
    o.id = -1;
}

select::reg::~reg(void)
{
    regs.release(id, version);
}

bool select::reg::valid(void) const
{
    return regs.valid(id, version);
}

void select::reg::clear(void)
{
    regs.release(id, version);
    id = -1;
}

select::reg& select::reg::operator = (const select::reg& o)
{
    regs.release(id, version);
    id = o.id;
    version = o.version;
    regs.acquire(id, version);
    return *this;
}

select::reg::operator int (void) const
{
    if (valid() == false)
    {
        if ((bool)reload == false)
        {
            //assert(0);
            id = -1;
        }
        else
        {
            auto v = reload();
            v.acquire();
            id = v.id;
            version = v.version;
        }
    }
    (regs.datas[id].binded == false) ? regs.frees.active(id) : regs.binds.active(id);
    return id;
}

void select::reg::acquire()
{
    regs.acquire(id, version);
}



select::select()
{
    for (int i = 0; i < 16; i++)
    {
        datas[i].id = i;
        datas[i].binded = false;
        datas[i].var = "";
        datas[i].ref = 0;
        datas[i].version = 1;

        frees.add(i);
    }
}

select::reg select::tmp(void)
{
    int v = frees.get();
    return alloc(datas[v]);
}

select::reg select::get(const std::string_view& name, std::function<reg (void)>&& reload)
{
    LOGT("try to get var %s", VIEW(name));
    auto it = vars.find(name);
    if (it != vars.end())
    {
        if (valid(it->second, it->first) == true)
        {
            LOGT("get var %s from binded %d:%d", VIEW(it->first), it->second, datas[it->second].version);
            auto r = alloc(datas[it->second], false);
            r.reload = std::move(reload);
            return r;
        }
        else
        {
            LOGT("get var %s from vars %d:%d, but it's invalid", VIEW(it->first), it->second, datas[it->second].version);
            vars.erase(it);
        }
    }
    auto r = reload();
    r.reload = std::move(reload);
    LOGT("get var %s -> %d", VIEW(name), r.id);
    return r;
}

select::reg select::var(const std::string_view& name)
{
    LOGT("try to get var %s", VIEW(name));
    auto it = vars.find(name);
    if (it != vars.end())
    {
        if (valid(it->second, it->first) == true)
        {
            LOGT("get var %s from binded %d:%d", VIEW(it->first), it->second, datas[it->second].version);
            return alloc(datas[it->second], false);
        }
        else
        {
            LOGT("get var %s from vars %d:%d, but it's invalid", VIEW(it->first), it->second, datas[it->second].version);
            vars.erase(it);
        }
    }

    int v = (frees.size > min_frees) ? frees.remove() : free_binds();
    datas[v].binded = true;
    datas[v].var = name;
    binds.add(v);
    vars.emplace(name, alloc(datas[v]));
    LOGT("bind %d:%d => %s", datas[v].id, datas[v].version, VIEW(name));
    return alloc(datas[v], false);
}

select::reg select::lock(void)
{
    int v = (frees.size > min_frees) ? frees.remove() : free_binds();
    locks.emplace(v);
    LOGT("lock %d:%d => %d", datas[v].id, datas[v].version, (int)locks.size());
    return alloc(datas[v]);
}

bool select::bind(const std::string_view& name, const reg& reg)
{
    auto& d = datas[reg.id];
    if ((d.binded == true) && (d.var.length() > 0) && (d.var != name))
    {
        unbind(d);
    }

    if (frees.remove(reg.id) == true)
    {
        binds.add(reg.id);
    }

    d.binded = true;
    d.var = name;

    LOGT("bind exists %d:%d => %s", d.id, d.version, VIEW(name));
    return vars.emplace(name, reg.id).second;
}

bool select::unbind(data& v)
{
    LOGT("unbinding %s of %d", VIEW(v.var), v.id);
    if (vars.erase(v.var) > 0)
    {
        if (binds.remove(v.id) == true)
        {
            frees.add(v.id);
            return unbind_impl(v);
        }
    }
    return false;
}

bool select::unbind_impl(data& v)
{
    v.var = "";
    v.binded = false;
    v.version += 1;
    LOGT("%s of %d unbinded", VIEW(v.var), v.id);
    return true;
}

void select::cleanup(bool var_only)
{
    LOGT("%d - %d - %d - %d", (int)locks.size(), (int)vars.size(), binds.size, frees.size);
    for (auto& it : vars)
    {
        unbind_impl(datas[it.second]);
        binds.remove(it.second);
        clear(datas[it.second]);
        frees.add(it.second);
    }
    vars.clear();
    if (var_only == false)
    {
    }
    LOGT("%d - %d - %d - %d", (int)locks.size(), (int)vars.size(), binds.size, frees.size);
}

int select::acquire(int id, uint32_t version)
{
    if (id < 0) return -1;
    if (version == datas[id].version)
    {
        datas[id].ref += 1;
        return id;
    }
    return -1;
}

int select::release(int id, uint32_t version)
{
    if (id < 0) return -1;
    data& v = datas[id];
    if (version == v.version)
    {
        v.ref -= 1;
        if (v.ref == 0)
        {
            clear(v);
            return -1;
        }
        return -1;
    }
    return -1;
}

bool select::valid(int id, const std::string_view& name)
{
    LOGT("check valid : %d, want : %s, real : %s", id, VIEW(name), VIEW(datas[id].var));
    return ((id >= 0) && (name == datas[id].var)) ? true : false;
}

bool select::valid(int id, uint32_t version)
{
    return ((id >= 0) && (version == datas[id].version)) ? true : false;
}

select::reg select::alloc(data& v, bool inc_ver)
{
    if (inc_ver == true)
    {
        v.version += 1;
    }
    return select::reg(v.id, v.version);
}

void select::clear(data& v)
{
    LOGT("clear %d:%d", v.id, v.version);
    v.version += 1;
    v.ref = 0;
    if (locks.erase(v.id) > 0)
    {
        frees.add(v.id);
    }
}

int select::free_binds(void)
{
    if (binds.size == 0)
    {
        return -1;
    }
    int v = binds.remove();
    datas[v].binded = false;
    clear(datas[v]);
    frees.add(v);
    return v;
}
