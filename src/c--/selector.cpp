#include "selector.h"

#include <log.h>

#include "common.h"


selector::selector()
{
    for (int i = 0; i < 256; i++)
    {
        datas[i].id = i;
        datas[i].status = 0;
        datas[i].var = "";
        //datas[i].ref = 0;
        datas[i].ver = 1;

        frees.add(i);
    }
}

const selector::reg selector::bind(const std::string_view& name)
{
    int v = frees.remove();
    data& r = datas[v];
    r.ver += 1;
    r.status = binded;
    r.var = name;
    int d = binds.add(v);
    if (d >= 0)
    {
        datas[d].ver += 1;
        datas[d].status = 0;
        frees.add(d);
    }
    return reg(&r);
}

const selector::reg selector::lock(void)
{
    int v = frees.remove();
    data& r = datas[v];
    r.ver += 1;
    r.status = binded;
    r.var = "";
    int d = locks.add(v);
    if (d >= 0)
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

bool selector::active(const reg& r)
{
    if (r.ver != r.ptr->ver) return false;

    switch (r.ptr->status)
    {
    case 0:
        return frees.active(r.ptr->id) != -1;
    case 1:
        return binds.active(r.ptr->id) != -1;
    case 2:
        return locks.active(r.ptr->id) != -1;
    default:
        return false;
    }
}

