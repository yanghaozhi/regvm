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
    const int                   count;
    const code_t*               codes;

    const int64_t               id;
    regvm_src_location          src;

    func(const code_t* codes, int count, int64_t id, const regvm_src_location* src);

    bool run(struct regvm* vm, int64_t offset = 0);

    static bool step(struct regvm* vm, const code_t* code, int offset, int max, int* next);
    //bool run(struct regvm* vm, const code_t* start, int count);
    //bool run(struct regvm* vm, uint64_t id, code_t code, int offset);

};

}


