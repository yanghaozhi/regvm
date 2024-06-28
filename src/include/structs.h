#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <map>
#include <deque>
#include <string>

#include <code.h>

namespace ext
{
    class var;
}


namespace core
{

template <typename T> struct regv;
template <typename T> struct var;

union uvalue
{
    typedef std::deque<var<ext::var>*>               list_t;
    typedef std::map<std::string, var<ext::var>*>    dict_t;

    int64_t         sint;
    uint64_t        uint;
    double          dbl;
    const char*     str;
    list_t*         list_v;
    dict_t*         dict_v;

    //double conv(int type, double v) const;
    //int64_t conv(int type, int64_t v) const;
    //uint64_t conv(int type, uint64_t v) const;
};

template <typename T> class var
{
protected:
    friend class error;

    int16_t         ref         = 1;

public:
    regv<T>*        reg         = NULL;

    uvalue          value;

    inline bool set_val(int type, uvalue val)
    {
#ifdef VAR_IMPL
        return static_cast<T*>(this)->set_val(type, val);
#else
        assert(0);
#endif
    }
    inline void set_reg(regv<T>* reg)
    {
#ifdef VAR_IMPL
        static_cast<T*>(this)->set_reg(reg);
#else
        assert(0);
#endif
    }

    inline void acquire(void)                   {++ref;};
    inline bool release(void)
    {
#ifdef VAR_IMPL
        return static_cast<T*>(this)->release();
#else
        assert(0);
        return false;
#endif
    }
};

template <typename T> struct regv
{
    uvalue          value;
    var<T>*         from;
    uint8_t         type;
    int8_t          idx;
    bool            need_free;

    inline bool clear()
    {
        store();

        if (need_free == true)
        {
            switch (type)
            {
            case TYPE_STRING:
                free((char*)value.str);
                value.str = NULL;
                break;
            case TYPE_LIST:
                delete value.list_v;
                break;
            case TYPE_DICT:
                delete value.dict_v;
                break;
            default:
                break;
            }
        }

        set_from(NULL);

        return true;
    }

    inline bool store() const
    {
        core::var<T>* v = from;
        if (v == NULL)
        {
            return false;
        }

        //if ((v->type != type) || (v->reg != idx))
        if (v->reg != this)
        {
            //ERROR(ERR_TYPE_MISMATCH, "store %d != %d", v->type, types[i]);
            return false;
        }

        //v->value = value;
        return v->set_val(type, value);
    }

    inline bool set_from(var<T>* v)
    {
        if (from != NULL)
        {
            from->set_reg(NULL);
        }
        if (v != NULL)
        {
            v->set_reg(this);
        }
        from = v;
        return true;
    }
};

};

