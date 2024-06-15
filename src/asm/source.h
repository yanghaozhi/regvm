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
    virtual ~source();

    virtual bool open(const char* name);

    virtual bool scan(void);

protected:
    FILE*           fp          = NULL;
    uint32_t        cur_line    = 0;
    std::string     file;

    std::unordered_map<std::string, int>        ids;

    virtual void comment(const char* line)
    {
        printf("\e[35m %s\e[0m", line);
    }

    virtual uint32_t setc(code_t& code, intptr_t* next, const char* str)    = 0;
    virtual int line(const code_t* code, int max_bytes, const char* orig)   = 0;
};

}

