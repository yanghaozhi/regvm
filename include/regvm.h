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

//rest表示当前指令后续还至少有多少条数据（某些指令需要附加数据）
//返回下一条指令的起始相对偏移（整数往后，负数往前）
//所有数字均以一条指令(2 byte)为单位
int regvm_exe_one(struct regvm* vm, const code_t* code, int rest);

//
//bool regvm_exe_pages(struct regvm* vm, const int pages_count, const code_page* pages);

#ifdef __cplusplus
};
#endif
