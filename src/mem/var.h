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

    var(uint8_t type, const char* name, const int len);
    ~var();

public:
    const uint16_t      type;
    const uint16_t      name_len;
    const uint32_t      hash;

    char                name[0];

    static var* create(uint8_t type, const char* name);
    static uint32_t calc_hash(const char* name, const int len);

    bool set_val(const core::regv& reg);
    bool set_reg(const core::regv* reg) const;
    bool release(void) const;

    bool store_from(core::regv& v);

    bool cmp(uint32_t key, const char* name, int len);
};

}
