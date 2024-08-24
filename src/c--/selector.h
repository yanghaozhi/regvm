#pragma once

#include <stdint.h>
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
        FREED,
        BINDED,
        LOCKED,
        FIXED,
    };

    struct data
    {
        uint8_t             id;
        uint8_t             status; //1 binds, 2 locks
        //uint16_t            ref;
        mutable uint32_t    ver;
        std::string_view    var;
    };

    struct reg
    {
        reg(void) : ver(0), ptr(NULL)   {};
        reg(const data* d) : ver(d->ver), ptr(d) {}

        uint32_t            ver;
        const data*         ptr;

        operator int (void) const   { return ptr->id; }
    };

    const reg fixed(int id);

    bool bind(const std::string_view& name, const reg& v);
    const reg bind(const std::string_view& name);

    bool lock(const reg& v);
    const reg lock(void);

    const reg tmp(void);

    bool release(const reg& r);
    bool active(const reg& r);
    bool valid(const reg& r);

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

    lru<uint8_t, 128>       frees;
    lru<uint8_t, 64>        binds;
    lru<uint8_t, 32>        locks;
};



