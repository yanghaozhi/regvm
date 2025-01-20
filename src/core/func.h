#pragma once

#include <stdint.h>

#include <map>

#include <irq.h>
#include <code.h>

#include "reg.h"


namespace core
{

struct func
{
    const bool                  need_free;
    const int32_t               id;

    const int32_t               count;      //code page count
    const code_t*               codes;      //code page begins

    regvm_src_location          src;

    func(const code_t* p, int32_t c, int32_t i, const regvm_src_location* l, int mode) :
        need_free((bool)mode), id(i), count(c), codes(p)
    {
        if (mode == VM_CODE_COPY)
        {
            void* n = malloc(sizeof(code_t) * count);
            memcpy(n, p, sizeof(code_t) * count);
            codes = (const code_t*)n;
        }

        if (l == NULL)
        {
            src.line = 0;
            src.file = NULL;
            src.func = NULL;
        }
        else
        {
            src = *l;
        }
    }

    ~func()
    {
        if (need_free == true)
        {
            free((void*)codes);
        }
    }
};

}


