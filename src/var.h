#pragma once

#include <stdint.h>
#include <code.h>


union uvalue
{
    int64_t             sint;
    uint64_t            uint;
    double              dbl;
    const char*         str;

    double conv(int type, double v) const;
    int64_t conv(int type, int64_t v) const;
    uint64_t conv(int type, uint64_t v) const;
};

class var
{
private:
    friend class error;

    int16_t             ref;

    var(uint8_t type, const char* name, const int len);
    ~var();

public:
    const uint16_t      type;
    int16_t             reg;
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

