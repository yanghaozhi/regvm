#pragma once

#include "labels.h"

#include <unordered_map>

namespace vasm
{

class strids : public labels
{
public:
    struct pass1 : public labels::pass1
    {
        pass1(strids& o) : labels::pass1(o), data(o)   {};
        virtual bool setc(code_t& code, intptr_t* next, const char* str);
    private:
        strids& data;
    };

    struct pass2 : public labels::pass2
    {
        pass2(strids& o);

        virtual void before();
        virtual void after();

        virtual int write_data(const void* data, int bytes)         = 0;
        virtual int write_code(const code_t* code, int bytes)       = 0;

        virtual bool setc(code_t& code, intptr_t* next, const char* str);

    private:
        strids& data;
    };


protected:
    uint32_t        str_id  = 0;
    std::unordered_map<std::string, uint32_t>   str_tab;
};

}
