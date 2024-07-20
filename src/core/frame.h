#pragma once

#include <code.h>
#include <irq.h>

#include <stdlib.h>

#include <list>
#include "func.h"
//#include "scope.h"

namespace core
{

class frame
{
    friend class error;

public:
    frame(frame& cur, func* func, int code, int reg, int ex, int offset);
    frame(regvm* vm, func* func, int code, int reg, int ex, int offset);
    ~frame();

    const uint32_t      depth;

    frame*              up      = NULL;
    frame*              down    = NULL;

    func*               running = NULL;
    regvm_src_location* cur     = NULL;

    const int64_t       id;

    bool run(int64_t entry = -1);

    //const code_t*       start   = NULL;     //first code pos
    //const code_t*       entry   = NULL;     //entry of current frame

    //void enter_block();
    //void leave_block();

    //var* add(const int type, const char* name);

    //var* get(const char* name) const;

private:
    regvm*              vm;
    const int           code;
    const int           reg;
    const int           ex;
    const int           offset;
    bool                valid   = true;

    int64_t gen_id(void);
//    scope&              globals;
//    std::list<scope>    scopes;
};


}

