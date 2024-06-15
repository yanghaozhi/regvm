#pragma once

#include <stdint.h>

#include <list>

#include "scope.h"

namespace ext
{

class var;

class mem
{
public:
    mem();
    ~mem();

    void enter_block();
    void leave_block();

    var* add(const int type, const char* name);

    var* get(const char* name) const;

private:
    scope               globals;
    std::list<scope>    scopes;
};

}
