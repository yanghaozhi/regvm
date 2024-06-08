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

//max表示当前指令后续还至少有多少字节（某些指令需要附加指令）
//返回实际读取了指令字节数
int regvm_exe_one(struct regvm* vm, const code_t* code, int max_bytes);

//
//bool regvm_exe_pages(struct regvm* vm, const int pages_count, const code_page* pages);

#ifdef __cplusplus
};
#endif
