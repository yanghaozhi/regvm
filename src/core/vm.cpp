#include "vm.h"

#include <code.h>

extern "C"
{

static bool vm_new(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    assert(0);
    return true;
}

static bool vm_store(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    assert(0);
    return true;
}

static bool vm_load(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    assert(0);
    return true;
}

static bool vm_block(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    return true;
}

static bool vm_call(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    return true;
}

struct regvm* regvm_init(struct regvm_ex* ext)
{
    auto vm = new regvm(ext);
    if (vm->handlers.init != NULL)
    {
        vm->handlers.init(vm);
    }
    return vm;
}

bool regvm_exit(struct regvm* vm)
{
    if (vm->handlers.exit != NULL)
    {
        vm->handlers.exit(vm);
    }
    delete vm;
    return true;
}

}

regvm::regvm(struct regvm_ex* ext) : reg()
{
    if (ext == NULL)
    {
        handlers.init = NULL;
        handlers.exit = NULL;

        handlers.vm_load = vm_load;
        handlers.vm_store = vm_store;
        handlers.vm_new = vm_new;
        handlers.vm_block = vm_block;
        handlers.vm_call = vm_call;
    }
    else
    {
        handlers = *ext;
    }
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
        ERROR(ERR_FUNCTION_CALL, *start, 0, "Can not get entry function");
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
        ERROR(ERR_FUNCTION_CALL, code, offset, "Can not get function info : %lu", id);
        return false;
    }

    core::frame f(*call_stack, &it->second, code, offset);
    return f.run();
}

//bool regvm::ret(void)
//{
//    if (call_stack->up == NULL)
//    {
//        return false;
//    }
//
//    auto cur = call_stack;
//    cur->up->down = NULL;
//
//    delete cur;
//
//    return true;
//}

