#pragma once

#include <stdbool.h>

#include "code.h"

struct regvm;

#ifdef __cplusplus
extern "C"
{
#endif

struct regvm_ex;

struct regvm* regvm_init(struct regvm_ex* ext);

bool regvm_exit(struct regvm* vm);


//执行指令数组
//从第一条指令开始执行，执行到末尾或者CODE_EXIT结束
//正常执行到末尾，以最终的$0寄存器的值为exit code
bool regvm_exec(struct regvm* vm, const code_t* code, int count, int64_t* exit);

//执行一条指令
//rest表示当前指令后续还至少有多少条数据（某些指令需要附加数据）
//返回下一条指令的起始相对偏移（整数往后，负数往前），0表示错误
int regvm_exec_step(struct regvm* vm, const code_t* code, int rest);


//计算指令长度
//单位：指令条数
int regvm_code_len(code_t code);

//
//bool regvm_exe_pages(struct regvm* vm, const int pages_count, const code_page* pages);

#ifdef __cplusplus
};
#endif

