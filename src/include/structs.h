#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <code.h>


namespace core
{

union uvalue
{
    int64_t         sint;
    uint64_t        uint;
    double          dbl;
    const char*     str;

    //double conv(int type, double v) const;
    //int64_t conv(int type, int64_t v) const;
    //uint64_t conv(int type, uint64_t v) const;
};

struct regv;

class var
{
protected:
    //var();
    //virtual ~var();

    friend class error;

    int16_t         ref         = 1;

public:
    regv*           reg         = NULL;

    uvalue          value;

    virtual void set_val(int type, uvalue val)  = 0;
    virtual void set_reg(regv*  reg)            = 0;

    inline void acquire(void)                   {++ref;};
    virtual bool release(void)                  = 0;
};

struct regv
{
    uvalue          value;
    var*            from;
    uint8_t         type;
    int8_t          idx;
    bool            need_free;

    bool clear();
    bool store() const;
    bool set_from(core::var* v);
};

};

