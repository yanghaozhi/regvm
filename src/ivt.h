#pragma once

#include <stdlib.h>
#include <stdint.h>

#include <utility>

struct isr
{
    void*   func;
    void*   arg;
};

struct ivt
{
    isr     isrs[16];

    ivt();

    bool set(uint32_t id, void* func, void* arg);

    template <typename T, typename ... Args> int call(struct regvm* vm, int id, Args ... args)
    {
        if (isrs[id].func == NULL)
        {
            return -1;
        }
        //return ((T)(isrs[id].func))(vm, id, std::forward<Args ...>(args ...));
        return ((T)(isrs[id].func))(vm, id, args ...);
    }



};
