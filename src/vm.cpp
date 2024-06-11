#include "vm.h"

extern "C"
{

struct regvm* regvm_init()
{
    auto vm = new regvm();
    return vm;
}

bool regvm_exit(struct regvm* vm)
{
    delete vm;
    return true;
}

}

regvm::regvm() : reg(), globals(0)
{
}

regvm::~regvm()
{
    while (ctx != NULL)
    {
        auto p = ctx;
        ctx = ctx->down;
        delete p;
    }
}

bool regvm::call(void* arg)
{
    auto next = new context(globals, ctx, arg);
    ctx = next;
    return true;
}

bool regvm::ret(void)
{
    if (ctx->up == NULL)
    {
        return false;
    }

    auto cur = ctx;
    cur->up->down = NULL;

    delete cur;

    return true;
}

