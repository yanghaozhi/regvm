#pragma once

#include <stdbool.h>

#include "code.h"

struct regvm;


#ifdef __cplusplus
extern "C"
{
#endif


union regvm_uvalue
{
    int64_t             sint;
    uint64_t            uint;
    double              dbl;
    const char*         str;
    void*               ptr;
};

struct regvm_reg_info
{
    int                 id;
    int                 ref;
    int                 type;
    int                 attr;
    union regvm_uvalue  value;
    void*               from;
};
typedef void (*reg_cb)(void* arg, const struct regvm_reg_info* info);
extern bool regvm_debug_reg_callback(struct regvm* vm, reg_cb cb, void* arg);

extern void regvm_debug_uvalue_print(int type, union regvm_uvalue uv);


struct regvm_var_info
{
    int                 ref;
    int                 type;
    int                 reg;
    int                 func_id;
    int                 call_id;
    int                 scope_id;
    int                 var_id;
    int                 attr;
    union regvm_uvalue  value;
    const char*         func_name;
    const char*         var_name;
    const void*         raw;
};
typedef void (*var_cb)(void* arg, const struct regvm_var_info* info);
extern bool regvm_debug_var_callback(struct regvm* vm, var_cb cb, void* arg);

#ifdef __cplusplus
};
#endif

