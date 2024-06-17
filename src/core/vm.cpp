#include "vm.h"

#include <code.h>

extern "C"
{

static bool vm_set(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    auto& r = vm->reg.id(code.reg);
    //if ((code.ex == TYPE_STRING) && (value & 0x01))
    //{
    //    auto& it = vm->idt.isrs[IRQ_STR_RELOCATE];
    //    if (it.func == NULL)
    //    {
    //        ERROR(ERR_STRING_RELOCATE, code, offset, "need to relocate string : %ld", value);
    //        return false;
    //    }
    //    value = it.call(vm, IRQ_STR_RELOCATE, code, offset, (void*)value);
    //    if (value == 0)
    //    {
    //        ERROR(ERR_STRING_RELOCATE, code, offset, "relocate string : %ld ERROR", value);
    //        return false;
    //    }
    //}
    return r.set(value, code.ex);
}

static bool vm_store(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    assert(0);
    //TODO
    //auto& r = vm->reg.id(code.reg);
    //if (code.ex == code.reg)
    //{
    //    return r.store();
    //}
    //else
    //{
    //    auto& e = vm->reg.id(code.ex);
    //    if ((e.type & 0x07) != TYPE_STRING)
    //    {
    //        ERROR(ERR_TYPE_MISMATCH, code, offset, "store name : %d", e.type);
    //        vm->fatal = true;
    //        return false;
    //    }
    //    else
    //    {
    //        var* v = vm->call_stack->add(r.type, e.value.str);
    //        return r.store(v);
    //    }
    //}
    return true;
}

static bool vm_load(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    assert(0);
    //TODO
    //auto& e = vm->reg.id(code.ex);
    //auto& r = vm->reg.id(code.reg);
    //return r.load(vm->call_stack->get(e.value.str));
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

        handlers.vm_set = vm_set;
        handlers.vm_load = vm_load;
        handlers.vm_store = vm_store;
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
    auto r = funcs.try_emplace((int64_t)0, start, count, 0, &src);
    if (r.second == false)
    {
        auto vm = this;
        ERROR(ERR_FUNCTION_CALL, *start, 0, "Can not get entry function");
        return false;
    }
    return call(0, code_t{0, 0, 0}, 0);
}

bool regvm::call(core::reg::v& reg, const code_t code, int offset)
{
    if (reg.type != TYPE_ADDR)
    {
        return call((int64_t)reg, code, offset);
    }
    else
    {
        auto o = call_stack;
        auto cur = o->running;
        core::frame f(call_stack, cur);
        call_stack = &f;

        bool rr = cur->run(this, (int64_t)reg);

        call_stack = o;
        return rr;
    }
}

bool regvm::call(int64_t id, const code_t code, int offset)
{
    //auto r = funcs.try_emplace(id, this, id, code, offset);
    //if ((r.second == false) || (r.first->second.codes == NULL) || (r.first->second.count == 0))
    //{
    //    auto vm = this;
    //    ERROR(ERR_FUNCTION_INFO, code, offset, "Can not get function info : %lu", id);
    //    return false;
    //}
    auto vm = this;
    auto it = funcs.find(id);
    if (it == funcs.end())
    {
        ERROR(ERR_FUNCTION_CALL, code, offset, "Can not get function info : %lu", id);
        return false;
    }

    core::frame f(call_stack, &it->second);
    auto o = call_stack;
    call_stack = &f;

    if (handlers.vm_call(this, code, offset, id) == false)
    {
        ERROR(ERR_FUNCTION_CALL, code, offset, "Can not get function info : %lu", id);
        return false;
    }

    bool rr = it->second.run(this);

    if (handlers.vm_call(this, code, offset, -id) == false)
    {
        ERROR(ERR_FUNCTION_CALL, code, offset, "Can not get function info : %lu", id);
        return false;
    }

    call_stack = o;
    return rr;
}

bool regvm::ret(void)
{
    if (call_stack->up == NULL)
    {
        return false;
    }

    auto cur = call_stack;
    cur->up->down = NULL;

    delete cur;

    return true;
}

