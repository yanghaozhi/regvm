#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "var.h"
#include <code.h>


class regs
{
private:
    uvalue      values[16];       //
    var*        froms[16];
    uint8_t     types[16];

    friend class error;

public:
    regs();
    ~regs();

    inline static bool valid_id(const int id)
    {
        return (id <= 0 || (id > 15)) ? false : true;
    }

    bool set(const int id, const code* inst);

    bool store(const int id);
    bool store(const int id, var* v);

    bool load(const int id, var* v);

    uint8_t type(const int id);
private:
};

