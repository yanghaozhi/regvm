#pragma once

#include <stdbool.h>

#include "code.h"

struct regvm;


#ifdef __cplusplus
extern "C"
{
#endif

struct regvm_reg_info
{
    int                 id;
    int                 type;
    union
    {
        int64_t         num;
        double          dbl;
        const char*     str;
    }                   value;
    void*               from;
};
typedef void (*reg_cb)(void* arg, const struct regvm_reg_info* info);
bool regvm_debug_reg_callback(struct regvm* vm, reg_cb cb, void* arg);


struct regvm_var_info
{
    int                 ref;
    int                 type;
    int                 reg;
    int                 scope;
    union
    {
        int64_t         num;
        double          dbl;
        const char*     str;
    }                   value;
    const char*         func;
    const char*         name;
};
typedef void (*var_cb)(void* arg, const struct regvm_var_info* info);
bool regvm_debug_var_callback(struct regvm* vm, var_cb cb, void* arg);

#ifdef __cplusplus
};
#endif

