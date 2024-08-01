#pragma once

//#include "debugger.h"
#include <set>

#include <regvm.h>
#include <debug.h>

#include "parser.h"

namespace vasm
{

class mem_run : public parser
{
public:
    mem_run(const char*);
    virtual ~mem_run();

    //void set_dbg(debugger* d);

    //virtual bool finish();

protected:
    std::set<std::string>   strs;

    virtual bool comment(const char* line, int size);
    virtual bool line(const char* str, inst* orig);

    //int64_t         code_bytes  = 0;
    //union
    //{
    //    code_t*     codes;
    //    void*       buf;
    //};
    //debugger*       dbg;

    //static void dump_reg_info(void* arg, const regvm_reg_info* info);
    //static int64_t debug_trap(regvm* vm, void*, code_t code, int offset, void* extra);
};

};

