#include <stdio.h>
#include <stdlib.h>


template <typename T> bool go(const char* name)
{
    T   op;
    if (op.open(name) == false)
    {
        return false;
    }

    typename T::pass1 s1(op);
    if (s1.scan() == false)
    {
        return false;
    }

    typename T::pass2 s2(op);
    if (s2.scan() == false)
    {
        return false;
    }

    if (op.finish() == false)
    {
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    return 0;
}
