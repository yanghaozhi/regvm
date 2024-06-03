#include "var.h"

var::var(uint8_t t) : type(t), reg(0), ref(0)
{
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
        delete this;
    }
}
