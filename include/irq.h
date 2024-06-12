#pragma once

#include <stdbool.h>

#include "code.h"


#ifdef __cplusplus
extern "C"
{
#endif

struct regvm;

struct regvm_src_location
{
    int                 line;
    const char*         file;
    const char*         func;
};

struct regvm_function
{
    uint64_t            id;

    regvm_src_location  entry;
    const code_t*       codes;
    int                 count;
};

struct regvm_error
{
    int                 code;
    const char*         reason;
    regvm_src_location* src;
};

enum ERR_CODE
{
    ERR_OK = 0,
    ERR_RUNTIME,
    ERR_INVALID_EX,
    ERR_INVALID_REG,
    ERR_TYPE_MISMATCH,
    ERR_INST_TRUNC,
    ERR_STRING_RELOCATE,
    ERR_FUNCTION_INFO,
};

//typedef int (*regvm_irq_error)(struct regvm* vm, int irq, int code, const char* reason);
//
//typedef int (*regvm_irq_src)(struct regvm* vm, int irq, struct src_location* loc);


enum IRQ
{
    IRQ_NOTHING     = 0,

    //遇到TRAP（调试）指令时发起
    //返回值为继续n条指令后再次自动发起，0表示不再发起
    IRQ_TRAP,               //extra always NULL

    //发生内部错误时发起
    IRQ_ERROR,              //extra is pointer of regvm_error

    //需要获取当前源码的文件名/行号/函数名
    IRQ_LOCATION,           //extra is pointer of regvm_src_location

    //重定位字符串地址
    //当调用SET指令时，如果类型为STRING，且指针值为奇数（合法指针值不会为奇数）
    //则会发起该中断以获得具体的真实地址
    IRQ_STR_RELOCATE,       //extra is string id

    //发起函数调用
    //需要在此中断中提供新函数的具体信息
    IRQ_FUNCTION_CALL,      //extra is token
};

//return 0 means FATAL ERROR, it will stop running !!!
typedef int64_t (*regvm_irq_handler)(struct regvm* vm, void* arg, int irq, code_t code, int offset, void* extra);

bool regvm_irq_set(struct regvm* vm, int irq, regvm_irq_handler func, void* arg);


#ifdef __cplusplus
};
#endif

