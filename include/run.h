#pragma once

#include <stdbool.h>

#include "code.h"

struct regvm;

#ifdef __cplusplus
extern "C"
{
#endif

struct regvm* regvm_init();

bool regvm_exit(struct regvm* vm);

//return the bytes has read
int regvm_exe_one(struct regvm* vm, const code2_t* code);

bool regvm_exe_pages(struct regvm* vm, const int pages_count, const code_page* pages);

#ifdef __cplusplus
};
#endif
