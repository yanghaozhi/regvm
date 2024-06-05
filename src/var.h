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
    uint16_t            name_len;
    uint32_t            hash;
    uvalue              value;

    char                name[];

    static var* create(uint8_t type, const char* name);
    static uint32_t calc_hash(const char* name, const int len);

    void set_reg(const int id);
    bool acquire(void);
    void release(void);

    bool cmp(uint32_t key, const char* name, int len);

};

class vars
{
public:
    vars();
    ~vars();

    bool add(var* v);

    var* get(const char* name);
    //var* get(uint32_t key, const char* name, int len);

    bool enter_block();
    bool exit_block();
};

