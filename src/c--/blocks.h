#pragma once

#include <list>
#include <unordered_map>

#include "common.h"
#include "selector.h"

class blocks
{
public:
    blocks(selector& s) : sel(s)    {};
    ~blocks()                       {};

    struct var
    {
        selector::reg   reg;
        const int       attr;
        const uint64_t  id;
    };

    const var* bind_var(const std::string_view& name, const selector::reg& v, int attr);
    const var* new_var(const std::string_view& name, int attr);
    template <typename F> const var* find_var(const std::string_view& name, int& attr, F reload)
    {
        for (auto& it : stack)
        {
            auto v = it.vars.find(name);
            if (v != it.vars.end())
            {
                if (sel.active(v->second.reg) == false)
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
        block(selector& s) : sel(s)    {};
        ~block();

        selector&       sel;
        std::unordered_map<std::string_view, var>   vars;
    };

    std::list<block>    stack;
    selector&           sel;
    static uint64_t     var_id;
};
