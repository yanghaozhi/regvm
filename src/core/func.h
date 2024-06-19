#pragma once

#include <stdint.h>

#include <map>

#include <irq.h>
#include <code.h>

namespace core
{

class func
{
public:
    const int32_t               id;

    const code_t*               codes;      //code page(file) begins
    const int64_t               count;      //code page count

    const int64_t               entry;      //function entry position
    const int64_t               size;       //function size

    regvm_src_location          src;

    func(const code_t* codes, int64_t count, int64_t entry, int64_t size, int32_t id, const regvm_src_location* src);

    bool run(struct regvm* vm, int64_t start = -1);

    static bool step(struct regvm* vm, const code_t* code, int offset, int max, int* next);
    //bool run(struct regvm* vm, const code_t* start, int count);
    //bool run(struct regvm* vm, uint64_t id, code_t code, int offset);

};

}


