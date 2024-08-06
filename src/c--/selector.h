#pragma once

#include <assert.h>

#include <set>
#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include <unordered_map>

#include "lru.h"


class selector
{
public:
    selector();

    enum STATUS
    {
        binded,
        locked,
    };

    struct data
    {
        uint8_t             id;
        uint8_t             status; //1 binds, 2 locks
        //uint16_t            ref;
        uint32_t            ver;
        std::string_view    var;
    };

    struct reg
    {
        reg(void) : ver(0), ptr(NULL)   {};
        reg(const data* d) : ver(d->ver), ptr(d) {}

        const uint32_t      ver;
        const data*         ptr;
    };

    const reg bind(const std::string_view& name);
    const reg lock(void);
    const reg tmp(void);

    void release(const reg& r);
    void active(const reg& r);

    //reg get(const std::string_view& name, std::function<reg (void)>&& reload);

    ////get a reg to store var
    //reg var(const std::string_view& name);
    ////get a reg and lock it
    //reg lock(void);

    //reg tmp(void);

    //bool bind(const std::string_view& name, const reg& reg);

    ////cleanup all binded/locked reg to frees
    //void cleanup(bool var_only);

private:
    data                    datas[256];

    lru<uint8_t, 256>       frees;
    lru<uint8_t, 64>        binds;
    lru<uint8_t, 64>        locks;
};



