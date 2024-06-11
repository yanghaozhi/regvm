#pragma once

#include <irq.h>

#include <stdlib.h>
#include <stdint.h>

#include <utility>



struct ivt
{
    struct isr
    {
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
