#pragma once

#include <stdbool.h>


#ifdef __cplusplus
extern "C"
{
#endif

struct regvm;

struct src_location
{
    int                 line;
    const char*         file;
    const char*         func;
};

typedef int (*regvm_irq_trap)(struct regvm* vm, int irq, int reg, int ex);

enum ERR_CODE
{
    ERR_OK = 0,
    ERR_TYPE_MISMATCH,
    ERR_INST_TRUNC,
};

typedef int (*regvm_irq_error)(struct regvm* vm, int irq, int code, const char* reason);

typedef int (*regvm_irq_src)(struct regvm* vm, int irq, struct src_location* loc);


enum IRQ
{
    IRQ_NOTHING     = 0,
    IRQ_TRAP,
    IRQ_ERROR,
    IRQ_LOCATION,
};

bool regvm_irq_set(struct regvm* vm, int irq, void* handler);


#ifdef __cplusplus
};
#endif

