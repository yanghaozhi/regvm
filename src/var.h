#pragma once

#include <stdint.h>


#pragma pack(1)
union uvalue
{
    int64_t             num;
    double              dbl;
    const char*         str;
};

class var
{
public:
    const uint8_t       type;
    uint8_t             reg;
    int16_t             ref;
    uvalue              value;

    var(uint8_t t);

    bool acquire(const int id = 0);
    void release(void);
};
#pragma pack()

