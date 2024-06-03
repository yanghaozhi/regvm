#pragma once

#include <stdint.h>

#include "var.h"


struct reg
{
    uint64_t    reg_vals[16];       //
    struct var* reg_vars[16];
    uint8_t     reg_types[16];
};
