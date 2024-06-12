#pragma once

#include <stdint.h>

#include <map>

#include <irq.h>

class func
{
public:
    regvm_function  info;

    func(struct regvm* vm, uint64_t id, code_t code, int offset);
    func(const code_t* start, int count);

    bool run(struct regvm* vm);

    static bool step(struct regvm* vm, const code_t* code, int offset, int max, int* next);
    //bool run(struct regvm* vm, const code_t* start, int count);
    //bool run(struct regvm* vm, uint64_t id, code_t code, int offset);

};

