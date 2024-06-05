#pragma once

#include <stdbool.h>

#include "code.h"

struct regvm;


#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*reg_cb)(int id, int64_t value, void* from, int type, void* arg);
bool regvm_debug_reg_callback(struct regvm* vm, reg_cb cb, void* arg);


typedef void (*var_cb)(int scope, const char* name, int64_t value, int type, int reg, int ref, void* arg);
bool regvm_debug_var_callback(struct regvm* vm, var_cb cb, void* arg);

#ifdef __cplusplus
};
#endif

