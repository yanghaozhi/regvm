#pragma once

#include <list>
#include <unordered_map>

#include "selector.h"

class blocks
{
public:
    blocks(selector& s) : sel(s)    {};
    ~blocks();

    const selector::reg new_var(const std::string_view& name);
    const selector::reg find_var(const std::string_view& name);

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
