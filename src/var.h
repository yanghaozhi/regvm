#pragma once

#include <stdint.h>


union uvalue
{
    int64_t             num;
    double              dbl;
    const char*         str;
};

struct var
{
public:
    uint8_t             type;
    uint8_t             reg;
    int16_t             ref;
    uvalue              value;

    uint64_t            key;
    char                name[];

    void init(uint8_t type, const char* name);

    bool acquire(const int id = 0);
    void release(void);

private:
    uint64_t hash(const char* name);
};

