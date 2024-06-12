#pragma once

#include <stdint.h>

#include <map>

#include <irq.h>

class func
{
public:
    struct obj
    {
        regvm_function  info;

        obj(struct regvm* vm, uint64_t id, code_t code, int offset);
        bool run(struct regvm* vm);
    };

    bool run(struct regvm* vm, uint64_t id, code_t code, int offset);

private:
    std::map<uint64_t, obj>   funcs;

};

