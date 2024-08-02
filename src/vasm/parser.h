#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>

#include <code.h>

#include "log.h"
#include "labels.h"


namespace vasm
{

class parser
{
public:
    std::string     file;

    parser();
    virtual ~parser();

    virtual bool go(const char* src);

    virtual int64_t size(void) const;

    virtual bool finish(FILE* fp, void (inst::*op)(FILE*) const);

    int             lineno  = 0;

protected:
    std::deque<inst*>                           insts;
    labels<std::string_view>                    label;

    virtual bool comment(const char* line, int size);
    virtual bool line(const char* str, inst* code);

private:
    const char* next_token(const char* str) const;
};

}

