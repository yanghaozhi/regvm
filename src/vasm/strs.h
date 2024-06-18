#pragma once

#include "labels.h"

#include <set>

namespace vasm
{

class strs : public labels
{
public:
    struct pass1 : public labels::pass1
    {
        pass1(strs& o) : labels::pass1(o)   {};
        virtual bool setc(code_t& code, intptr_t* next, const char* str);
    };

    struct pass2 : public labels::pass2
    {
        pass2(strs& o);

        virtual int write_code(const code_t* code, int bytes)     = 0;

        virtual bool setc(code_t& code, intptr_t* next, const char* str);

    private:
        strs& data;
    };


protected:
    std::set<std::string>     str_tab;
};

}

