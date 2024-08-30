#pragma once

#include <stdint.h>
#include <code.h>

#define VAR_IMPL    ext::var

#include "../include/structs.h"


#include "log.h"

namespace ext
{


class var : public core::var
{
private:
    friend class scope;

public:
    var(uint8_t type, uint64_t id);
    virtual ~var();

    const uint64_t      id;

    int                 reload  = -1;

    inline bool set_val(const core::regv& reg);
    inline bool set_reg(const core::regv* reg) const;
    inline bool release(void) const
    {
        if (--ref > 0)
        {
            LOGD("var %p ref : %d", this, ref);
            return true;
        }
        LOGD("var %p ref : %d", this, ref);

        if (ref == 0)
        {
            //delete this;
            delete this;
            //this->~var();
            //free((void*)this);
        }

        return false;
    }

    static bool set_val(core::var* v, const core::regv& r)          {return v->set_val(r);};
    static bool set_reg(const core::var* v, const core::regv* r)    {return v->set_reg(r);};
    static bool release(const core::var* v)                         {return v->release();};

    bool store_from(core::regv& v);
};

}
