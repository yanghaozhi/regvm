#pragma once

#include <stdint.h>
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

class var
{
protected:
    //var();
    //virtual ~var();

    int16_t         ref         = 1;

public:
    int16_t         reg         = -1;
    uint16_t        type;

    uvalue          value;

    virtual void set_val(int type, uvalue val)  = 0;
    virtual void set_reg(const int id)          = 0;

    inline void acquire(void)                   {++ref;};
    virtual bool release(void)                  = 0;
};

struct regv
{
    uvalue          value;
    var*            from;
    uint8_t         type;
    int8_t          idx;

    bool store() const;
    bool set_from(core::var* v);
};

};

