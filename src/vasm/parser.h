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

    virtual ~parser();

    virtual bool open(char* data, int64_t size);
    //virtual bool open(const char* name);

    virtual bool go(const char* src);

    //virtual bool finish(FILE* fp);

    int             lineno  = 0;

protected:
    std::unordered_map<std::string_view, inst*> ids;
    std::deque<inst*>                           insts;
    labels<std::string_view>                    label;

    virtual bool comment(const char* line, int size);
    virtual bool line(const char* str, inst* orig);

private:
    int             fd      = -1;
    int64_t         size    = -1;

    const char* next_token(const char* str) const;
};

}

