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

bool regvm::run(const code_t* start, int count)
{
    auto r = funcs.try_emplace(0, start, count);
    if (r.second == false)
    {
        auto vm = this;
        ERROR(ERR_FUNCTION_INFO, *start, 0, "Can not get entry function");
        return false;
    }
    ctx = new context(globals, ctx, &r.first->second);
    return r.first->second.run(this);
}

bool regvm::call(uint64_t id, code_t code, int offset)
{
    auto r = funcs.try_emplace(id, this, id, code, offset);
    if ((r.second == false) || (r.first->second.info.codes == NULL) || (r.first->second.info.count == 0))
    {
        auto vm = this;
        ERROR(ERR_FUNCTION_INFO, code, offset, "Can not get function info : %lu", id);
        return false;
    }
    ctx = new context(globals, ctx, &r.first->second);
    return r.first->second.run(this);
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

