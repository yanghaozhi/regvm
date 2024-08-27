#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <map>
#include <deque>
#include <string>

#include <code.h>

#include "mlib.h"

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

namespace core
{

class var;
struct regv;

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

void free_uvalue(int type, uvalue v);

class var
{
protected:
    friend class error;

    mutable int16_t ref         = 1;

public:
    uvalue          value;
    mutable regv*   reg         = NULL;

    const char* operator = (const char* v)
    {
        value.str = strdup(v);
        return v;
    }

    inline void acquire(void)                   {++ref;};

    virtual bool set_val(const regv& r)         {return false;};
    virtual bool set_reg(const regv* r) const   {return false;};
    virtual int vtype(void) const               {return 0;};
    virtual bool release(void) const            {return false;};
};

struct regv
{
    uvalue                  value;
    mutable var*            from;
    uint8_t                 type;
    uint8_t                 idx;
    mutable bool            need_free;

    inline bool clear()
    {
        //store();

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
        core::var* v = from;
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

    inline bool load(const var* v)
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

        type = v->vtype();
        value = v->value;
        set_from(v);
        need_free = false;

        return true;
    }

    inline bool set_from(const var* v) const
    {
        if (v != NULL)
        {
            v->set_reg(this);
        }
        if (from != NULL)
        {
            from->set_reg(NULL);
        }
        from = const_cast<decltype(from)>(v);
        return true;
    }
};

inline void free_uvalue(int type, uvalue v)
{
    switch (type)
    {
    case TYPE_STRING:
        free(v.ptr);
        break;
    case TYPE_LIST:
        for (auto& it : *v.list_v)
        {
            it->release();
        }
        delete v.list_v;
        break;
    case TYPE_DICT:
        for (auto& it : *v.dict_v)
        {
            it.second->release();
        }
        delete v.dict_v;
        break;
    default:
        break;
    }
}

};

