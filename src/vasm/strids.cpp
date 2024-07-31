#include "strids.h"

#include <regvm.h>

using namespace vasm;


bool strids::calc_id_len(uint64_t id, code_t& code, intptr_t* next)
{
    //TODO
    //if (id <= 0x7FFF)
    //{
    //    code.id = CODE_SETS;
    //    *next = (uint16_t)id;
    //}
    //else if (id <= 0x7FFFFFFF)
    //{
    //    code.id = CODE_SETI;
    //    *next = (uint32_t)id;
    //}
    //else
    //{
    //    code.id = CODE_SETL;
    //    *next = (uint64_t)id;
    //}
    return true;
}
