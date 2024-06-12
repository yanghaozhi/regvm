#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "var.h"
#include <code.h>


class regs
{
public:
    struct reg_v
    {
        uvalue          value;
        var*            from;
        uint8_t         type;
        int8_t          idx;

        inline operator double () const    {return conv_d(type);};
        inline operator int64_t () const   {return conv_i(type);};
        inline operator uint64_t () const  {return conv_u(type);};

        bool set(uint64_t num, int ex);

        bool store() const;
        bool store(var* v);

        bool load(var* v);
        
        reg_v& neighbor(int id);

        bool set_from(var* v);

        double conv_d(int type) const;
        int64_t conv_i(int type) const;
        uint64_t conv_u(int type) const;
    };


    friend class error;

    regs();
    ~regs();

    inline reg_v& id(int i)
    {
#ifdef DEBUG
        if (i < 0 || (i >= size))
        {
            assert(0);
        }
#endif
        return values[i];
    }

    //uint8_t type(const int id);

private:
    static const int    size = 16;
    reg_v               values[size];

};

