#pragma once

#include <stdint.h>
#include <code.h>
#include <debug.h>

#define VAR_IMPL    ext::var

#include "../include/structs.h"


#include "log.h"

namespace ext
{


class var : public core::var
{
private:
    friend bool ::regvm_debug_var_callback(struct regvm* vm, var_cb cb, void* arg);

public:
    var(uint8_t type, uint64_t id = (uint64_t)-1);
    virtual ~var();

    const uint64_t      id;

    int                 reload  = -1;

    inline bool set_val(const core::regv& reg);
    inline bool set_reg(const core::regv* reg) const;
    inline bool release(void) const
    {
        LOGD("release var %016llx - %d - %p", (long long)id, ref, this);
        if (--ref > 0)
        {
            LOGD("var %p ref : %d", this, ref);
            return true;
        }

        if (ref == 0)
        {
            //delete this;
            delete this;
            //this->~var();
            //free((void*)this);
        }

        return false;
    }

    static bool set_val(core::var* v, const core::regv& r);
    static bool release(const core::var* v);
    static bool reg_chg_from(const core::regv* r, const core::var* cur, const core::var* next);

    bool store_from(core::regv& v);
};

}
