#include "select.h"


class select               regs;

select::reg::reg(int i, uint32_t v) : id(i), version(v)
{
    id = regs.acquire(id, version);
}

select::reg::reg(void) : id(-1), version(0)
{
}

select::reg::reg(const reg& o) : id(regs.acquire(o.id, o.version)), version(o.version)
{
}

select::reg::reg(reg&& o) : id(o.id), version(o.version)
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

void select::reg::acquire()
{
    regs.acquire(id, version);
}



select::select()
{
    for (int i = 0; i < 16; i++)
    {
        datas[i].id = i;
        datas[i].locked = false;
        datas[i].ref = 0;
        datas[i].version = 1;

        frees.add(i);
    }
}

select::reg select::get(void)
{
    int v = frees.get();
    return alloc(datas[v]);
}

select::reg select::var(const std::string_view& name)
{
    auto it = vars.find(name);
    if (it != vars.end())
    {
        return select::reg(it->second->id, it->second->version);
    }

    int v = (frees.size > min_frees) ? frees.remove() : free_binds();
    binds.add(v);
    vars.emplace(name, &datas[v]);
    return alloc(datas[v]);
}

select::reg select::lock(void)
{
    int v = (frees.size > min_frees) ? frees.remove() : free_binds();
    locks.emplace(v);
    return alloc(datas[v]);
}

void select::cleanup(bool var_only)
{
    for (auto& it : vars)
    {
        binds.remove(it.second->id);
        cleanup(*it.second);
        frees.add(it.second->id);
    }
    if (var_only == true)
    {
        return;
    }
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
            cleanup(v);
            return -1;
        }
        return -1;
    }
    return -1;
}

bool select::valid(int id, uint32_t version)
{
    return ((id >= 0) && (version == datas[id].version)) ? true : false;
}

select::reg select::alloc(data& v)
{
    return select::reg(v.id, ++v.version);
}

void select::cleanup(data& v)
{
    v.version += 1;
    v.locked = false;
    if (v.var.length() > 0)
    {
        vars.erase(v.var);
    }
}

int select::free_binds(void)
{
    if (binds.size == 0)
    {
        return -1;
    }
    int v = binds.remove();
    cleanup(datas[v]);
    frees.add(v);
    return v;
}
