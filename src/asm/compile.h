#pragma once

#include "source.h"

namespace vasm
{

class compile : public source
{
public:
    bool find_label(const char* str, std::string& label);
    int64_t label_id(const std::string& label);

protected:
    uint32_t    code_size       = 0;
    int64_t     cur_label_id    = 0;

    std::unordered_map<std::string, int64_t>        label_ids;

    virtual int write_code(const code_t* code, int bytes)     = 0;

    virtual void comment(const char* line);
    virtual uint32_t setc(code_t& code, intptr_t* next, const char* str);
    virtual int line(const code_t* code, int max_bytes, const char* orig);
};

}

