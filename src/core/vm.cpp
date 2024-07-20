#include "vm.h"

#include <code.h>

#include "ext.h"

extern "C"
{

struct regvm* regvm_init(void)
{
    auto vm = new REGVM_IMPL();
    CRTP_CALL(vm_init);
    return vm;
}

bool regvm_exit(struct regvm* vm)
{
    CRTP_CALL(vm_exit);
    delete vm;
    return true;
}

}

regvm::regvm() : reg()
{
}

regvm::~regvm()
{
    while (call_stack != NULL)
    {
        auto p = call_stack;
        call_stack = call_stack->down;
        delete p;
    }
}

bool regvm::run(const code_t* start, int count)
{
    regvm_src_location src = {0, "NULL", "..."};
    auto r = funcs.try_emplace((int32_t)0, start, count, 0, count, 0, &src);
    if (r.second == false)
    {
        auto vm = this;
        VM_ERROR(ERR_FUNCTION_CALL, *start, 0, "Can not get entry function");
        return false;
    }
    core::frame f(this, &r.first->second, code_t{0, 0, 0}, 0);
    return f.run();
}

bool regvm::call(core::reg::v& reg, const code_t code, int offset)
{
    if (reg.type != TYPE_ADDR)
    {
        return call((int64_t)reg, code, offset);
    }
    else
    {
        core::frame f(*call_stack, call_stack->running, code, offset);
        return f.run((int64_t)reg);
    }
}

bool regvm::call(int64_t id, const code_t code, int offset)
{
    auto vm = this;
    auto it = funcs.find(id);
    if (it == funcs.end())
    {
        VM_ERROR(ERR_FUNCTION_CALL, code, offset, "Can not get function info : %lu", id);
        return false;
    }

    core::frame f(*call_stack, &it->second, code, offset);
    return f.run();
}


#ifndef REGVM_EXT
bool regvm_core::vm_init()
{
    return true;
}

bool regvm_core::vm_exit()
{
    return true;
}

core::var* regvm_core::vm_var(int id)
{
    assert(0);
    return NULL;
}

core::var* regvm_core::vm_var(int type, const char* name)
{
    assert(0);
    return NULL;
}

bool regvm_core::vm_new(const code_t code, int offset, int64_t value)
{
    assert(0);
    return true;
}

bool regvm_core::vm_store(const code_t code, int offset, int64_t value)
{
    assert(0);
    return true;
}

bool regvm_core::vm_load(const code_t code, int offset, int64_t value)
{
    assert(0);
    return true;
}

bool regvm_core::vm_block(const code_t code, int offset, int64_t value)
{
    return true;
}

bool regvm_core::vm_call(const code_t code, int offset, int64_t value)
{
    return true;
}
#endif
