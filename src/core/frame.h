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
    frame(frame& cur, func* func, code_t code, int offset);
    frame(regvm* vm, func* func, code_t code, int offset);
    ~frame();

    const uint16_t      depth;

    frame*              up      = NULL;
    frame*              down    = NULL;

    func*               running = NULL;
    regvm_src_location* cur     = NULL;

    const int64_t       id;

    //core::reg::page<core::reg::SIZE>    sub_func;

    enum REASON
    {
        ERROR   = 0,
        RET,
        EXIT,
        END,
    };

    int run(void);

    static bool one_step(struct regvm* vm, const code_t code, int max, int* next, const void* extra);

    //const code_t*       start   = NULL;     //first code pos
    //const code_t*       entry   = NULL;     //entry of current frame

    //void enter_block();
    //void leave_block();

    //var* add(const int type, const char* name);

    //var* get(const char* name) const;

private:
    regvm*              vm;
    struct 
    {
        code_t          code;
        int             offset;
    }                   caller;
    bool                valid   = true;
    enum REASON         reason;
    int                 flow;

    inline int64_t gen_id(void) const;
//    scope&              globals;
//    std::list<scope>    scopes;

    inline int step(struct regvm* vm, code_t inst, int offset, int max, const void* extra);
};


}

