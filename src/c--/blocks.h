#pragma once

#include <list>
#include <unordered_set>
#include <unordered_map>

#include "inst.h"
#include "common.h"
#include "selector.h"

class blocks
{
public:
    blocks(insts_t* i, selector& s) : insts(i), regs(s) {}
    ~blocks()   {}

    struct var
    {
        var(const selector::reg& r, int a, uint64_t i) : reg(r), attr(a), id(i) {}

        selector::reg   reg;
        const int       attr;
        const uint16_t  id;
    };

    const var* bind_arg(const std::string_view& name, int r, int attr);
    const var* bind_var(const std::string_view& name, const selector::reg& v, int attr);
    const var* new_var(const std::string_view& name, int attr);
    template <typename F> const var* find_var(const std::string_view& name, int& attr, F reload)
    {
        for (auto& it : stack)
        {
            auto v = it.vars.find(name);
            if (v != it.vars.end())
            {
                if (regs.active(v->second.reg) == false)
                {
                    LOGW("find invalid result %d:%d for %s, need to reload it", (int)v->second.reg, v->second.reg.ver, VIEW(name));
                    v->second.reg = reload(v->second.attr, v->second.id);
                    return &v->second;
                }
                LOGT("find result %d:%d for %s", (int)v->second.reg, v->second.reg.ver, VIEW(name));
                attr = v->second.attr;
                return &v->second;
            }
        }
        return NULL;
    }

    bool enter();
    bool leave();

private:
    struct block
    {
        block(blocks* b) : first(b->var_id), cur(b)     {};
        ~block();

        const int       first;
        int             last        = -1;
        blocks*         cur;
        std::unordered_map<std::string_view, var>   vars;
    };

    uint16_t            var_id      = 1;
    insts_t*            insts;
    selector&           regs;
    std::list<block>    stack;

    const var* add_2_vars(const std::string_view& name, const selector::reg& v, int attr);
};
