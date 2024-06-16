#pragma once

#include "compile.h"


namespace vasm
{

class mem_2_run : public compile
{
public:
    mem_2_run();
    virtual ~mem_2_run();

    struct pass1 : public compile::pass1
    {
        pass1(mem_2_run& o);

        virtual int write_code(const code_t* code, int bytes);
    private:
        mem_2_run& data;
    };

    struct pass2 : public compile::pass2
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
};

};

