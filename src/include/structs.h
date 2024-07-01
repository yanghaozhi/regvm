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
template <typename T> struct var_crtp;

union uvalue
{
    typedef std::deque<var*>               list_t;
    typedef std::map<std::string, var*>    dict_t;

    int64_t         sint;
    uint64_t        uint;
    double          dbl;
    const char*     str;
    void*           ptr;
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

    mutable int16_t ref         = 1;

public:
    uvalue          value;

    template <typename T> var_crtp<typename T::var_t>* crtp()
    {
        return static_cast<var_crtp<typename T::var_t>*>(this);
    }
};


template <typename T> class var_crtp : public var
{
public:
    mutable regv<T>*    reg     = NULL;

#define NOTHING
#ifdef REGVM_EXT
#define CRTP_FUNC(name, ext, ret, argc, ...)                                        \
    inline ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__)) ext         \
    {                                                                               \
        return static_cast<ext T*>(this)->name(MLIB_CALL_LIST(argc, __VA_ARGS__));  \
    }
#else
#define CRTP_FUNC(name, ret, argc, ...)                                             \
    inline ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__)) ext         \
    {                                                                               \
        assert(0);                                                                  \
    }
#endif

    CRTP_FUNC(set_val,  NOTHING, bool, 1, const regv<T>&);
    CRTP_FUNC(set_reg,  const, bool, 1, const regv<T>*);
    CRTP_FUNC(release,  NOTHING, bool, 0);

#undef CRTP_FUNC
#undef NOTHING

    inline void acquire(void)                   {++ref;};
};

template <typename T> struct regv
{
    uvalue                  value;
    mutable var_crtp<T>*    from;
    uint8_t                 type;
    int8_t                  idx;
    mutable bool            need_free;

    inline bool clear()
    {
        store();

        if (need_free == true)
        {
            free_uvalue(type, value);
            value.ptr = NULL;
        }

        set_from(NULL);

        return true;
    }

    inline bool store() const
    {
        core::var_crtp<T>* v = from;
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

    inline bool load(const var_crtp<T>* v)
    {
        if (v->reg == this)
        {
            value = v->value;
            return true;
        }

        clear();

        if (v->reg != NULL)
        {
            //auto o = neighbor(&r, reg->idx);
            v->reg->clear();
        }

        type = static_cast<const T*>(v)->type;
        value = v->value;
        set_from(v);
        need_free = false;

        return true;
    }

    inline bool set_from(const var_crtp<T>* v) const
    {
        if (from != NULL)
        {
            from->set_reg(NULL);
        }
        if (v != NULL)
        {
            v->set_reg(this);
        }
        from = const_cast<decltype(from)>(v);
        return true;
    }
};

};

