#pragma once

#include <stdint.h>


#pragma pack(1)
struct var
{
    uint8_t             type;
    uint8_t             reg;
    uint16_t            ref;
    union
    {
        int64_t         num;
        double          dbl;
        const char*     str;
    };
};
#pragma pack()

