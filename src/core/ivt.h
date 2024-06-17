#pragma once

#include <irq.h>

#include <stdlib.h>
#include <stdint.h>

#include <utility>


namespace core
{

struct ivt
{
    struct isr
    {
        int8_t              id;
        static const int    DO_NOT_CHECK = -128;
        int8_t              err_ret;  //-128 means DO NOT check
        regvm_irq_handler   func;
        void*               arg;
        regvm_irq_handler   def_func;

        int64_t call(struct regvm* vm, int id, code_t code, int offset, void* extra);
    };

    isr     isrs[16];

    ivt();

    bool set(uint32_t id, regvm_irq_handler func, void* arg);

    int64_t call(struct regvm* vm, int id, code_t code, int offset, void* args, int64_t defval = 0);
};

}

