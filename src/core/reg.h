#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "structs.h"
#include <code.h>

namespace ext
{
    class var;
}


namespace core
{

class reg
{
public:
    struct v : public core::regv<ext::var>
    {
        inline operator double () const    {return conv_d(type);};
        inline operator int64_t () const   {return conv_i(type);};
        inline operator uint64_t () const  {return conv_u(type);};

        bool write(uint64_t num, int type, bool clear);

        //bool clear();

        //bool store() const;
        //bool store(core::var* v);

        //bool load(core::var* v);
        //
        //v& neighbor(int id);

        //bool set_from(core::var* v);

        inline double conv_d(int type) const
        {
            switch (type)
            {
            case TYPE_SIGNED:
                return (double)value.sint;
            case TYPE_UNSIGNED:
                return (double)value.uint;
            case TYPE_DOUBLE:
                return value.dbl;
#ifdef NULL_AS_0
            case TYPE_NULL:
                return 0;
#endif
            default:
                assert(0);
                return -1;
            }
        }

        inline int64_t conv_i(int type) const
        {
            switch (type)
            {
            case TYPE_SIGNED:
            case TYPE_UNSIGNED:
            case TYPE_ADDR:
                return value.sint;
            case TYPE_DOUBLE:
                return (int64_t)value.dbl;
#ifdef NULL_AS_0
            case TYPE_NULL:
                return 0;
#endif
            default:
                assert(0);
                return -1;
            }
        }

        inline uint64_t conv_u(int type) const
        {
            switch (type)
            {
            case TYPE_SIGNED:
            case TYPE_UNSIGNED:
            case TYPE_ADDR:
                return value.uint;
            case TYPE_DOUBLE:
                return (uint64_t)value.dbl;
#ifdef NULL_AS_0
            case TYPE_NULL:
                return 0;
#endif
            default:
                assert(0);
                return -1;
            }
        }
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

