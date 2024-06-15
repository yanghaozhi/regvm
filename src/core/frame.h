#pragma once

#include <code.h>
#include <irq.h>

#include <stdlib.h>

#include <list>
#include "func.h"
//#include "scope.h"

struct var;

class frame
{
    friend class error;

public:
    frame(frame* ctx, func* func);
    ~frame();

    frame*              up      = NULL;
    frame*              down    = NULL;

    func*               running = NULL;
    regvm_src_location* cur     = NULL;

    //const code_t*       start   = NULL;     //first code pos
    //const code_t*       entry   = NULL;     //entry of current frame

    //void enter_block();
    //void leave_block();

    //var* add(const int type, const char* name);

    //var* get(const char* name) const;

//private:
//    scope&              globals;
//    std::list<scope>    scopes;
};

