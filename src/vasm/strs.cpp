#include "strs.h"

#include <regvm.h>

using namespace vasm;

bool strs::pass1::setc(code_t& code, intptr_t* next, const char* str)
{
    //TODO
    //if (labels::pass1::setc(code, next, str) == false)
    //{
    //    code.id = CODE_SETL;
    //    code.ex = TYPE_STRING;
    //}
    return true;
}

strs::pass2::pass2(strs& o) : labels::pass2(o), data(o)
{
}

bool strs::pass2::setc(code_t& code, intptr_t* next, const char* str)
{
    //TODO
    //if (labels::pass2::setc(code, next, str) == false)
    //{
    //    code.id = CODE_SETL;
    //    code.ex = TYPE_STRING;
    //    auto it = data.str_tab.emplace(str).first;
    //    *next = (intptr_t)it->c_str();
    //}
    return true;
}

