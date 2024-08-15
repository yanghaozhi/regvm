#pragma once

#include <stdint.h>
#include <code.h>

#define VAR_IMPL    ext::var

#include "../include/structs.h"



namespace ext
{


class var : public core::var
{
private:
    friend class scope;

public:
    var(uint8_t type, uint64_t id);
    ~var();

    const uint16_t      type;

    const uint64_t      id;


    bool set_val(const core::regv& reg);
    bool set_reg(const core::regv* reg) const;
    bool release(void) const;

    virtual int vtype(void) const                {return type;}

    bool store_from(core::regv& v);
};

}
