#pragma once

#include "labels.h"
#include "strs.h"
#include "debugger.h"

#include <regvm.h>
#include <debug.h>

namespace vasm
{

class mem_2_run : public strs
{
public:
    mem_2_run(const char*);
    virtual ~mem_2_run();

    void set_dbg(debugger* d);

    struct pass1 : public strs::pass1
    {
        pass1(mem_2_run& o);

        virtual int write_code(const code_t* code, int bytes);
    private:
        mem_2_run& data;
    };

    struct pass2 : public strs::pass2
    {
        pass2(mem_2_run& o);

        virtual int write_code(const code_t* code, int bytes);
    private:
        mem_2_run&  data;
        code_t*     cur         = NULL;
    };

    virtual bool finish();

protected:
    int64_t         code_bytes  = 0;
    union
    {
        code_t*     codes;
        void*       buf;
    };
    debugger*       dbg;

    static void dump_reg_info(void* arg, const regvm_reg_info* info);
    static int64_t debug_trap(regvm* vm, void*, code_t code, int offset, void* extra);
};

};

