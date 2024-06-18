#pragma once

#include <map>
#include <string>

#include <regvm.h>
#include <debug.h>

#include "strids.h"

namespace vasm
{

class bin_file : public strids
{
public:
    bin_file(const char* n) : bin(n)    {};

    virtual bool open(const char* name);

    virtual bool finish();

    struct pass1 : public strids::pass1
    {
        pass1(strids& o) : strids::pass1(o)   {};
    };

    struct pass2 : public strids::pass2
    {
        pass2(bin_file& o);
        virtual ~pass2();

        virtual void before();
        virtual void after();

        virtual int write_data(const void* data, int bytes);
        virtual int write_code(const code_t* code, int bytes);

    private:
        strids& data;
        int         fd = -1;
    };

private:
    std::string                     bin;
    std::map<uint32_t, const char*> strs;

    //static int64_t debug_trap(regvm* vm, void*, code_t code, int offset, void* extra);
    static int64_t str_relocate(regvm* vm, void* arg, code_t code, int offset, void* extra);
};

};

