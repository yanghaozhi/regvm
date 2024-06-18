#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "structs.h"
#include <code.h>


namespace core
{

class reg
{
public:
    struct v : public core::regv
    {
        inline operator double () const    {return conv_d(type);};
        inline operator int64_t () const   {return conv_i(type);};
        inline operator uint64_t () const  {return conv_u(type);};

        bool set(uint64_t num, int ex);

        //bool clear();

        //bool store() const;
        //bool store(core::var* v);

        //bool load(core::var* v);
        //
        //v& neighbor(int id);

        //bool set_from(core::var* v);

        double conv_d(int type) const;
        int64_t conv_i(int type) const;
        uint64_t conv_u(int type) const;
    };


    friend class error;

    reg();
    ~reg();

    inline v& id(int i)
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
    v                   values[size];

};


}

