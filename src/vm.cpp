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

regvm::regvm() : reg(), globals(), ctx(new context(globals))
{
}

regvm::~regvm()
{
    delete ctx;
}

bool regvm::call(void)
{
    auto next = new context(globals, ctx);
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

