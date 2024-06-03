#pragma once

#include <stdbool.h>

#include "code.h"

struct regvm;


#ifdef __cplusplus
extern "C"
{
#endif

bool regvm_debug_reg_info(struct regvm* vm, int id, uint64_t* value, void** vars, int* type);

#ifdef __cplusplus
};
#endif

