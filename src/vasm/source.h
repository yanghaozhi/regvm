#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include <code.h>


namespace vasm
{

class source
{
public:
    FILE*           fp          = NULL;
    std::string     file;

    virtual ~source();

    virtual bool open(const char* name);

    virtual bool finish();

protected:
    struct pass
    {
        source&     src;
        int64_t     cur_line    = 0;

        pass(source& s) : src(s)  {}

        bool scan(void);

        virtual void comment(const char* line)
        {
            printf("\e[35m %s\e[0m", line);
        }
        virtual bool setc(code_t& code, intptr_t* next, const char* str)   = 0;
        virtual bool line(const code_t* code, int max_bytes, const char* orig)  = 0;
    };

    std::unordered_map<std::string, int>        ids;
};

}

