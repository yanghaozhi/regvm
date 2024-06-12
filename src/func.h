#pragma once

#include <stdint.h>

#include <map>

class func
{
public:
    const code_t*   codes;
    int             count;

private:
    std::map<uint64_t, func*>   func_tab;
};

