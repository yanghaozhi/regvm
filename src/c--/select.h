#pragma once

#include <assert.h>

#include <set>
#include <vector>
#include <string>
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

        inline operator int (void) const
        {
            if (valid() == false)
            {
                assert(0);
            }
            return id;
        }
        template <typename F> int get(F func)
        {
            if (valid() == false)
            {
                func(this);
                acquire();
            }
            return id;
        }
    private:
        int             id;
        uint32_t        version;

        friend class select;

        void acquire();
    };

    //get a reg to store var
    reg var(const std::string_view& name);

    //get a reg and lock it
    reg lock(void);

    reg get(void);

    //cleanup all binded/locked reg to frees
    void cleanup(bool var_only);

private:
    struct data
    {
        int8_t              id;
        bool                locked;
        uint16_t            ref;
        uint32_t            version;
        std::string_view    var;
    };
    data                    datas[16];
    const int               min_frees   = 6;

    lru<int8_t, 16>         frees;
    lru<int8_t, 16>         binds;

    std::set<int>           locks;
    std::unordered_map<std::string_view, data*>    vars;

    reg alloc(data& v);
    void cleanup(data& v);
    int free_binds(void);

    int acquire(int id, uint32_t version);
    int release(int id, uint32_t version);
    bool valid(int id, uint32_t version);
};

extern class select               regs;


