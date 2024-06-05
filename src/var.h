#pragma once

#include <stdint.h>


union uvalue
{
    int64_t             num;
    double              dbl;
    const char*         str;
};

class var
{
private:
    int16_t             ref;

    var(uint8_t type, const char* name, const int len);
    ~var();

public:
    const uint8_t       type;
    uint8_t             reg;
    const uint16_t      name_len;
    const uint32_t      hash;

    uvalue              value;

    char                name[0];

    static var* create(uint8_t type, const char* name);
    static uint32_t calc_hash(const char* name, const int len);

    void set_reg(const int id);
    inline void acquire(void)   {++ref;}
    bool release(void);

    bool cmp(uint32_t key, const char* name, int len);
};

