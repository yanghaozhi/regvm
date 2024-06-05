#pragma once

#include "reg.h"
#include "context.h"

class error
{
public:
    template <typename T> void reg_info(const regs& reg, T cb)
    {
        //cb(vm->reg.values[i].num, vm->reg.froms[i], vm->reg.types[i]);
        const int size = sizeof(reg.types);
        for (int i = 1; i < size; i++)
        {
            cb(i, reg.values[i].num, reg.froms[i], reg.types[i]);
        }
    }

    template <typename T> void ctx_vars(const context& ctx, T cb)
    {
        auto f = [cb](int id, var* v)
        {
            if (v != NULL)
            {
                cb(id, v, v->ref);
            }
        };
        ctx.globals.for_each(f);

        int id = 1;
        for (auto it = ctx.scopes.rbegin(); it != ctx.scopes.rend(); ++it)
        {
            it->for_each(f);
            ++id;
        }
    }

};
