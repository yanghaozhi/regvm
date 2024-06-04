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
    int8_t              ref;
    uint8_t             name_len;
    uvalue              value;

    uint64_t            key;
    char                name[];

    void init(uint8_t type, const char* name);

    bool acquire(const int id = 0);
    void release(void);

    bool cmp(uint64_t key, const char* name, int len);

private:
    uint64_t hash(const char* name);
};

class vars
{
public:
    vars();
    ~vars();

    bool add(var* v);

    var* get(uint64_t key, const char* name, int len);

    bool enter_block();
    bool exit_block();
};
