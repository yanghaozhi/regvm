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
    };

    isr     isrs[16];

    ivt();

    bool set(uint32_t id, regvm_irq_handler func);

    int call(struct regvm* vm, int id, code_t code, int offset, void* args);


};
