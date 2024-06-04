#include "var.h"

#include <stdlib.h>
#include <string.h>


void var::init(uint8_t t, const char* n)
{
    type = t;
    strcpy(name, n);
    key = hash(n);
}

uint64_t var::hash(const char* name)
{
    const int len = strlen(name);
    const int step = sizeof(uint64_t);
    const char* p = name;
    uint64_t h = 0;
    int i = 0;
    for (i = 0; len - i >= step; i += step)
    {
        h += *(uint64_t*)p;
        p += step;
    }

    switch (len - i)
    {
    case 7:
        h += *(uint64_t*)p;
        break;
    case 6:
    case 5:
        h += *(uint32_t*)p;
        h += (*(uint16_t*)(p + sizeof(uint32_t)) << sizeof(uint32_t));
        break;
    case 4:
    case 3:
        h += *(uint32_t*)p;
        break;
    case 2:
    case 1:
        h += *(uint16_t*)p;
        break;
    }
    return h;
}

bool var::acquire(const int id)
{
    if (id != 0)
    {
        if (reg != 0)
        {
            return false;
        }
        reg = id;
    }

    ++ref;

    return true;
}

void var::release(void)
{
    if (--ref <= 0)
    {
        //delete this;
        free(this);
    }
}
