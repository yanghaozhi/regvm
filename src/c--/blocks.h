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

    bool bind_var(const std::string_view& name, const selector::reg& v, int attr);
    const selector::reg new_var(const std::string_view& name, int attr);
    template <typename F> const selector::reg find_var(const std::string_view& name, int& attr, F reload)
    {
        for (auto& it : stack)
        {
            auto v = it.vars.find(name);
            if (v != it.vars.end())
            {
                if (sel.active(v->second.r) == false)
                {
                    LOGW("find invalid result %d:%d for %s, need to reload it", (int)v->second.r, v->second.r.ver, VIEW(name));
                    return reload(v->second.attr);
                }
                LOGT("find result %d:%d for %s", (int)v->second.r, v->second.r.ver, VIEW(name));
                attr = v->second.attr;
                return v->second.r;
            }
        }
        return selector::reg();
    }

    bool enter();
    bool leave();

private:
    struct var
    {
        selector::reg   r;
        int             attr;
    };
    struct block
    {
        block(selector& s) : sel(s)    {};
        ~block();

        selector&       sel;
        std::unordered_map<std::string_view, var>   vars;
    };
    std::list<block>    stack;
    selector&           sel;
};
