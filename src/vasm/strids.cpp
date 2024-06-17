#include "strids.h"

#include <regvm.h>

using namespace vasm;

bool strids::pass1::setc(code_t& code, intptr_t* next, const char* str)
{
    if (labels::pass1::setc(code, next, str) == true)
    {
        return true;
    }

    code.id = CODE_SETL;
    code.ex = TYPE_STRING;
    auto it = data.str_tab.find(str);
    if (it == data.str_tab.end())
    {
        auto id = ++data.str_id;
        it = data.str_tab.try_emplace(str, (id << 1) + 1).first;
    }
    *next = (intptr_t)it->first.c_str();
    return true;
}

void strids::pass2::before()
{
    uint64_t end = 0;
    for (auto& it : data.str_tab)
    {
        uint32_t id = it.second;
        write_data(&id, sizeof(uint32_t));
        write_data(it.first.c_str(), it.first.length() + 1);
        printf("remap %u - %s\n", id, it.first.c_str());
    }
    write_data(&end, sizeof(uint64_t));
}

void strids::pass2::after()
{
}

strids::pass2::pass2(strids& o) : labels::pass2(o), data(o)
{
}

bool strids::pass2::setc(code_t& code, intptr_t* next, const char* str)
{
    if (labels::pass2::setc(code, next, str) == false)
    {
        auto it = data.str_tab.find(str);
        if (it == data.str_tab.end())
        {
            return false;
        }

        code.ex = TYPE_STRING;
        if (it->second <= 0x7FFF)
        {
            code.id = CODE_SETS;
            *next = (uint16_t)it->second;
        }
        else if (it->second <= 0x7FFFFFFF)
        {
            code.id = CODE_SETS;
            *next = (uint32_t)it->second;
        }
        else
        {
            code.id = CODE_SETS;
            *next = (uint64_t)it->second;
        }
    }
    return true;
}

