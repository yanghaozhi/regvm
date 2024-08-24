#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "structs.h"
#include <code.h>

#include "log.h"

namespace ext
{
    class var;
}


namespace core
{

class reg
{
public:
    struct v : public core::regv
    {
        inline operator double () const    {return conv<double>(type);};
        inline operator int64_t () const   {return conv<int64_t>(type);};
        inline operator uint64_t () const  {return conv<uint64_t>(type);};

        inline bool write(uint64_t v, int t, bool c)
        {
            if ((c == true) || (need_free == true))
            {
                clear();
            }

            need_free = false;
            type = t;
            value.uint = v;

            return true;
        }

        template <typename T> inline T conv(int type) const
        {
            switch (type)
            {
            case TYPE_SIGNED:
                return (T)value.sint;
            case TYPE_UNSIGNED:
                return (T)value.uint;
            case TYPE_DOUBLE:
                return (T)value.dbl;
#ifdef NULL_AS_0
            case TYPE_NULL:
                return (T)0;
#endif
            default:
                assert(0);
                return (T)-1;
            }
        }

        inline void copy(const v& o)
        {
            clear();
            need_free = false;
            type = o.type;
            value = o.value;
        }
    };

    int     flow    = 0;

    friend class error;

    reg()
    {
        memset(values, 0, sizeof(values));
        for (int i = 0; i < SIZE; i++)
        {
            values[i].idx = i;
        }
    }

    ~reg()
    {
        for (int i = 0; i < SIZE; i++)
        {
            if (values[i].from != NULL)
            {
                values[i].from->release();
                //values[i].set_from(NULL);
            }
            if(values[i].need_free == true)
            {
                free_uvalue(values[i].type, values[i].value);
            }
        }
    }

    inline v& id(int i)
    {
#ifdef DEBUG
        if (i < 0 || (i >= SIZE))
        {
            assert(0);
        }
#endif
        return values[ ((i >> 7) * flow) + i ];
    }

    //uint8_t type(const int id);

private:
    static const int    SIZE = 256;
    v                   values[SIZE];

};


}

