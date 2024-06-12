#pragma once

#include <code.h>
#include <irq.h>

#include <stdlib.h>

#include <list>
#include "scope.h"

struct var;

class context
{
    friend class error;

public:
    context(scope& globals, context* cur, void* arg);
    ~context();

    context*            up      = NULL;
    context*            down    = NULL;

    regvm_src_location  func;
    regvm_src_location* cur     = NULL;

    //const code_t*       start   = NULL;     //first code pos
    //const code_t*       entry   = NULL;     //entry of current context

    void enter_block();
    void leave_block();

    var* add(const int type, const char* name);

    var* get(const char* name) const;

private:
    scope&              globals;
    std::list<scope>    scopes;
};

