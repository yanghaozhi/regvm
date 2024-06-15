#pragma once

#include <stdint.h>
#include <code.h>

#include "../include/structs.h"



class var : public core::var
{
private:
    friend class error;

    var(uint8_t type, const char* name, const int len);
    ~var();

public:
    const uint16_t      type;
    const uint16_t      name_len;
    const uint32_t      hash;

    char                name[0];

    static var* create(uint8_t type, const char* name);
    static uint32_t calc_hash(const char* name, const int len);

    virtual void set_val(int type, core::uvalue val);
    virtual void set_reg(const int id);
    virtual bool release(void);

    bool cmp(uint32_t key, const char* name, int len);
};

