#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <map>
#include <deque>
#include <string>

#include <code.h>

#include "mlib.h"


namespace core
{

struct var;
template <typename T> struct regv;
template <typename T> struct var_type;

union uvalue
{
    typedef std::deque<var*>               list_t;
    typedef std::map<std::string, var*>    dict_t;

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

extern void free_uvalue(int type, uvalue v);

struct var
{
protected:
    friend class error;

    int16_t         ref         = 1;

public:
    uvalue          value;

    template <typename T>   var_type<typename T::var_t>* crtp()
    {
        return static_cast<var_type<typename T::var_t>*>(this);
    }
};


template <typename T> class var_type : public var
{
public:
    regv<T>*        reg         = NULL;

#ifdef REGVM_EXT
#define CRTP_FUNC(name, ret, argc, ...)                                             \
    inline ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__))             \
    {                                                                               \
        return static_cast<T*>(this)->name(MLIB_CALL_LIST(argc, __VA_ARGS__));      \
    }
#else
#define CRTP_FUNC(name, ret, argc, ...)                                             \
    inline ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__))             \
    {                                                                               \
        assert(0);                                                                  \
    }
#endif

    CRTP_FUNC(set_val,  bool, 1, regv<T>&);
    CRTP_FUNC(set_reg,  bool, 1, regv<T>*);
    CRTP_FUNC(release,  bool, 0);

#undef CRTP_FUNC

    inline void acquire(void)                   {++ref;};
};

template <typename T> struct regv
{
    uvalue          value;
    var_type<T>*    from;
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

    inline bool store()
    {
        core::var_type<T>* v = from;
        if (v == NULL)
        {
            return false;
        }

        if (v->reg != this)
        {
            //ERROR(ERR_TYPE_MISMATCH, "store %d != %d", v->type, types[i]);
            return false;
        }

        return v->set_val(*this);
    }

    inline bool set_from(var_type<T>* v)
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

