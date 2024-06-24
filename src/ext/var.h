#pragma once

#include <stdint.h>
#include <code.h>

#define VAR_IMPL    ext::var

#include "../include/structs.h"



namespace ext
{


class var : public core::var<var>
{
private:
    friend class scope;

    var(uint8_t type, const char* name, const int len);
    ~var();

public:
    const uint16_t      name_len;
    const uint32_t      hash;

    char                name[0];

    static var* create(uint8_t type, const char* name);
    static uint32_t calc_hash(const char* name, const int len);

    void set_val(int type, core::uvalue val);
    void set_reg(core::regv<var>* reg);
    bool release(void);

    core::regv<var>* neighbor(core::regv<var>* r, int id);

    bool store(core::regv<var>& v);
    bool load(core::regv<var>& v);

    bool cmp(uint32_t key, const char* name, int len);
};

}
