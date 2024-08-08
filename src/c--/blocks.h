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

    bool bind_var(const std::string_view& name, const selector::reg& v);
    const selector::reg new_var(const std::string_view& name);
    template <typename F> const selector::reg find_var(const std::string_view& name, F reload)
    {
        for (auto& it : stack)
        {
            auto v = it.vars.find(name);
            if (v != it.vars.end())
            {
                if (sel.active(v->second) == false)
                {
                    LOGW("find invalid result %d:%d for %s, need to reload it", (int)v->second, v->second.ver, VIEW(name));
                    return reload();
                }
                LOGT("find result %d:%d for %s", (int)v->second, v->second.ver, VIEW(name));
                return v->second;
            }
        }
        return selector::reg();
    }

    bool enter();
    bool leave();

private:
    struct block
    {
        block(selector& s) : sel(s)    {};
        ~block();

        selector&       sel;
        std::unordered_map<std::string_view, selector::reg> vars;
    };
    std::list<block>    stack;
    selector&           sel;
};
