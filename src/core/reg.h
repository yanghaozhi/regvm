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
    friend class error;
    friend class ::regvm;

    static const int            SIZE    = 32;
    template <int SIZE> struct  page;
    typedef struct page<SIZE>   page_t;

    struct v : public core::regv
    {
        inline operator double () const    {return conv<double>(type);};
        inline operator int64_t () const   {return conv<int64_t>(type);};
        inline operator uint64_t () const  {return conv<uint64_t>(type);};

        inline bool write(uint64_t v, int t);
        template <typename T> inline T conv(int type) const;
        inline void copy(const v& o);
    };

    template <int SIZE> struct page
    {
        page()  { memset(values, 0, sizeof(v) * SIZE); }
        ~page() { cleanup(0, SIZE); }

        inline void prepare();
        inline void cleanup(const int begin, const int end);
        inline int id(const v& r);

        v           values[SIZE];
    };

    inline v& id(int i)
    {
#ifdef DEBUG
        if (i < 0 || (i >= SIZE * 2))
        {
            assert(0);
        }
#endif
        return (i < SIZE) ? pages[0]->values[i] : pages[1]->values[i - SIZE];
    }

    inline int idx(const v& r)
    {
        auto i = pages[0]->id(r);
        if (i >= 0) return i;

        i = pages[1]->id(r);
        if (i >= 0) return i + SIZE;

        return -1;
    }

private:
    page<SIZE>*     pages[2];
};

inline bool reg::v::write(uint64_t v, int t)
{
    //if ((c == true) || (need_free == true))
    //{
    //    clear();
    //}

    need_free = false;
    type = t;
    value.uint = v;

    return true;
}

template <typename T> inline T reg::v::conv(int type) const
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

inline void reg::v::copy(const v& o)
{
    clear();
    need_free = false;
    type = o.type;
    value = o.value;
}

template <int SIZE> inline void reg::page<SIZE>::prepare()
{
    //LOGT("prepare %p", this);
    //memset(values, 0, sizeof(v) * SIZE);
}

template <int SIZE> inline void reg::page<SIZE>::cleanup(const int begin, const int end)
{
    LOGT("cleanup %p", this);
    for (int i = begin; i < end; i++)
    {
        if (unlikely(values[i].from != NULL))
        {
            values[i].set_from(NULL);
        }
        if (unlikely(values[i].need_free == true))
        {
            free_uvalue(values[i].type, values[i].value);
        }
    }
    //memset(&values[begin], 0, sizeof(v) * (end - begin));
}

template <int SIZE> inline int reg::page<SIZE>::id(const v& r)
{
    auto i = &r - values;
    return ((0 <= i) && (i < SIZE)) ? i : -1;
}

}

