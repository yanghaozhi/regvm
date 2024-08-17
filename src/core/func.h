#pragma once

#include <stdint.h>

#include <map>

#include <irq.h>
#include <code.h>

#include "reg.h"

#define UNSUPPORT_TYPE(op, t, c, o, ...) VM_ERROR(ERR_TYPE_MISMATCH, c, o, "UNSUPPORT %s value type : %d", op, t); 

namespace core
{

typedef int (*vm_sub_op_t)(regvm* vm, int a, int b, int c, int offset);

class func
{
public:
    const int32_t               id;

    const int32_t               count;      //code page count
    const code_t*               codes;      //code page begins

    regvm_src_location          src;

    func(const code_t* codes, int32_t count, int32_t id, const regvm_src_location* src);

    int run(struct regvm* vm);

    static bool one_step(struct regvm* vm, const code_t code, int max, int* next, const void* extra);
};

}


