#include <run.h>

#include <stdlib.h>

struct regvm
{
};

struct regvm* regvm_init()
{
    return NULL;
}

bool regvm_exit(struct regvm* vm)
{
    return false;
}

bool regvm_exe_one(struct regvm* vm, const struct code* inst)
{
    return false;
}

