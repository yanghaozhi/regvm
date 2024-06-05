#pragma once

#include <code.h>

#include <list>
#include "scope.h"

struct var;

class context
{
public:
    context();
    ~context();

    void enter_block();
    void leave_block();

    var* add(const code* inst);

    var* get(const char* name);

private:
    std::list<scope>    scopes;
};
