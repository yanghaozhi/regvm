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
    virtual ~var();

    const uint16_t      type;

    const uint64_t      id;

    int                 reload  = -1;

    virtual bool set_val(const core::regv& reg) override;
    virtual bool set_reg(const core::regv* reg) const override;
    virtual bool release(void) const override;

    virtual int vtype(void) const override                {return type;}

    bool store_from(core::regv& v);
};

}
