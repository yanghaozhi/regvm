#pragma once

#include <stdint.h>
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
    ERR_INVALID_VAR,
    ERR_INVALID_CODE,
    ERR_TYPE_MISMATCH,
    ERR_INST_TRUNC,
    ERR_STRING_RELOCATE,
    ERR_FUNCTION_CALL,
};

//typedef int (*regvm_irq_error)(struct regvm* vm, int irq, int code, const char* reason);
//
//typedef int (*regvm_irq_src)(struct regvm* vm, int irq, struct src_location* loc);


enum IRQ
{
    IRQ_NOTHING     = 0,

    //遇到TRAP（调试）指令时发起
    //返回值为继续n条指令后再次自动发起，0表示不再发起
    IRQ_TRAP,               //extra is pointer of regvm_src_location

    //发生内部错误时发起
    IRQ_ERROR,              //extra is pointer of regvm_error

    //需要获取当前源码的文件名/行号/函数名
    IRQ_LOCATION,           //extra is pointer of regvm_src_location

    //重定位字符串
    IRQ_STR_RELOCATE,       //extra is string id
};

typedef int64_t (*regvm_irq_handler)(struct regvm* vm, void* arg, code_t code, int offset, void* extra);

bool regvm_irq_set(struct regvm* vm, int irq, regvm_irq_handler func, void* arg);


#ifdef __cplusplus
};
#endif

