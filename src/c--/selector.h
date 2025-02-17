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

    static const int        SIZE    = 32;

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

    bool set_fixed(int reserved);

    const reg fixed(int id);

    bool bind(const std::string_view& name, const reg& v);
    const reg bind(const std::string_view& name);

    bool lock(const reg& v);
    const reg lock(void);

    const reg tmp(void);

    bool release(const reg& r);
    bool active(const reg& r);
    bool valid(const reg& r);


    int unused(void) const;

private:
    data                    datas[SIZE];

    int                     reserved    = -1;

    lru<uint8_t, SIZE>      frees;
    lru<uint8_t, SIZE/2>    binds;
    lru<uint8_t, SIZE/4>    locks;
};



