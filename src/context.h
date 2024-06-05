#pragma once

#include <code.h>

#include <stdlib.h>

#include <list>
#include "scope.h"

struct var;

class context
{
    friend class error;

public:
    context(scope& globals, context* cur = NULL);
    ~context();

    context*            up;
    context*            down;

    void enter_block();
    void leave_block();

    var* add(const code* inst, uint8_t type);

    var* get(const char* name);

private:
    scope&              globals;
    std::list<scope>    scopes;
};

