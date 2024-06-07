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

    src_location*       cur;
    src_location        func;

    context*            up;
    context*            down;

    void enter_block();
    void leave_block();

    var* add(const code8_t* code);

    var* get(const char* name) const;

private:
    scope&              globals;
    std::list<scope>    scopes;
};

