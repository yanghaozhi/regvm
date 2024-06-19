#pragma once

#include <stdint.h>

#include <map>
#include <list>

#include <debug.h>

#include "scope.h"

namespace ext
{

class var;

class mem
{
public:
    mem();
    ~mem();

    var* add(const int type, const char* name);

    var* get(const char* name) const;

    bool block(int64_t frame, int ex);

    bool call(int64_t func);

    void dump(var_cb cb, void* arg) const;

private:
    struct context
    {
        context(int64_t frame);
        ~context();

        void enter_block();
        void leave_block();

        const int64_t       frame;
        std::list<scope>    scopes;
    };

    scope                   globals;
    std::list<context>      frames;
};

}
