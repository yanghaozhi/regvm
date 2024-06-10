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
};

//typedef int (*regvm_irq_error)(struct regvm* vm, int irq, int code, const char* reason);
//
//typedef int (*regvm_irq_src)(struct regvm* vm, int irq, struct src_location* loc);


enum IRQ
{
    IRQ_NOTHING     = 0,
    IRQ_TRAP,               //extra always NULL
    IRQ_ERROR,              //extra is pointer of regvm_error
    IRQ_LOCATION,           //extra is pointer of regvm_src_location
};

//return 0 means FATAL ERROR, it will stop running !!!
typedef int (*regvm_irq_handler)(struct regvm* vm, int irq, code_t code, int offset, void* extra);

bool regvm_irq_set(struct regvm* vm, int irq, regvm_irq_handler func);


#ifdef __cplusplus
};
#endif

