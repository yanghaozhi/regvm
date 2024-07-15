#pragma once

#include <assert.h>

#include <set>
#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include <unordered_map>

#include "lru.h"


class select
{
public:
    select();

    class reg
    {
    public:
        reg(int id, uint32_t ver);
        reg(void);
        reg(const reg& o);
        reg(reg&& o);
        ~reg(void);

        bool valid(void) const;
        void clear(void);

        reg& operator = (const reg& o);

        operator int (void) const;
    private:
        mutable int                 id;
        mutable uint32_t            version;
        std::function<reg (void)>   reload;

        friend class select;

        void acquire();
    };

    template <typename F> reg get(const std::string_view& name, F reload)
    {
        auto it = vars.find(name);
        if (it != vars.end())
        {
            auto ret = it->second;
            if (ret.valid() == true)
            {
                ret.reload = reload;
                return ret;
            }
            else
            {
                vars.erase(it);
            }
        }
        auto r = reload();
        r.reload = reload;
        return r;
    }

    //get a reg to store var
    reg var(const std::string_view& name);
    //get a reg and lock it
    reg lock(void);

    reg tmp(void);

    bool bind(const std::string_view& name, const reg& reg);

    //cleanup all binded/locked reg to frees
    void cleanup(bool var_only);

private:
    struct data
    {
        int8_t              id;
        bool                binded;
        uint16_t            ref;
        uint32_t            version;
        std::string_view    var;
    };
    data                    datas[16];
    const int               min_frees   = 6;

    lru<int8_t, 16>         frees;
    lru<int8_t, 16>         binds;

    std::set<int>           locks;
    std::unordered_map<std::string_view, select::reg>    vars;

    reg alloc(data& v);
    void clear(data& v);
    int free_binds(void);

    int acquire(int id, uint32_t version);
    int release(int id, uint32_t version);
    bool valid(int id, uint32_t version);
};

extern class select               regs;


