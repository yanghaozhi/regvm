#include "selector.h"

#include <log.h>

#include "common.h"


selector::selector()
{
    for (unsigned int i = 0; i < sizeof(datas) / sizeof(datas[0]); i++)
    {
        datas[i].id = i;
        datas[i].status = 0;
        datas[i].var = "";
        //datas[i].ref = 0;
        datas[i].ver = 1;

        if ((int)i < frees.max)
        {
            frees.add(i);
        }
    }
}

const selector::reg selector::fixed(int id)
{
    if (id < frees.max) return reg();
    return reg(&datas[id]);
}

bool selector::bind(const std::string_view& name, const selector::reg& v)
{
    if (valid(v) == false)
    {
        LOGE("reg %d:%p is invalid", v.ver, v.ptr);
        return false;
    }

    data& r = datas[v.ptr->id];
    if (r.status != FREED)
    {
        LOGW("reg %d:%d - %d is NOT freed", r.id, r.ver, r.status);
        return false;
    }

    r.status = BINDED;
    r.var = name;
    int d = binds.add(v);
    if (d < 0)
    {
        LOGW("Can NOT add %d to binds", r.id);

        datas[d].ver += 1;
        datas[d].status = FREED;
        frees.add(d);
        return false;
    }
    else
    {
        frees.remove(r.id);
    }

    LOGT("bind %d:%d -> %s", v.ver, v.ptr->id, VIEW(name));
    return true;
}

const selector::reg selector::bind(const std::string_view& name)
{
    int v = frees.remove();
    data& r = datas[v];
    r.ver += 1;
    r.status = BINDED;
    r.var = name;
    int d = binds.add(v);
    if (d < 0)
    {
        datas[d].ver += 1;
        datas[d].status = 0;
        frees.add(d);
    }
    return reg(&r);
}

bool selector::lock(const selector::reg& v)
{
    if (valid(v) == false)
    {
        LOGE("reg %d:%p is invalid", v.ver, v.ptr);
        return false;
    }

    data& r = datas[v.ptr->id];
    if (r.status != FREED)
    {
        LOGW("reg %d:%d - %d is NOT freed", r.id, r.ver, r.status);
        return false;
    }

    r.status = LOCKED;
    r.var = "";
    int d = locks.add(v);
    if (d < 0)
    {
        LOGW("Can NOT add %d to locks", r.id);

        datas[d].ver += 1;
        datas[d].status = FREED;
        frees.add(d);
        return false;
    }
    else
    {
        frees.remove(r.id);
    }

    LOGT("lock %d:%d", v.ver, v.ptr->id);
    return true;
}

const selector::reg selector::lock(void)
{
    int v = frees.remove();
    data& r = datas[v];
    r.ver += 1;
    r.status = LOCKED;
    r.var = "";
    int d = locks.add(v);
    if (d < 0)
    {
        datas[d].ver += 1;
        datas[d].status = 0;
        frees.add(d);
    }
    return reg(&r);
}

const selector::reg selector::tmp(void)
{
    int v = frees.get();
    data& r = datas[v];
    r.ver += 1;
    return reg(&r);
}

bool selector::release(const reg& r)
{
    if (valid(r) == false) return false;

    r.ptr->ver += 1;

    switch (r.ptr->status)
    {
    case 0:
        return true;
    case 1:
        binds.remove(r);
        return frees.active(r) != -1;
    case 2:
        locks.remove(r);
        return frees.add(r) != -1;
    default:
        return false;
    }
}

bool selector::active(const reg& r)
{
    if (valid(r) == false) return false;

    switch (r.ptr->status)
    {
    case FREED:
        return frees.active(r.ptr->id) != -1;
    case BINDED:
        return binds.active(r.ptr->id) != -1;
    case LOCKED:
        return locks.active(r.ptr->id) != -1;
    default:
        return false;
    }
}

bool selector::valid(const reg& r)
{
    return ((r.ptr != NULL) && (r.ptr->ver == r.ver)) ? true : false;
}
