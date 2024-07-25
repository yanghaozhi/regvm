#include "vm.h"

#include <code.h>

#include "ext.h"


extern int vm_CODE_NOP(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_TRAP(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_CLEAR(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_LOAD(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_STORE(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_GLOBAL(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_NEW(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_BLOCK(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_CONV(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_CHG(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_CMP(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_JUMP(regvm* vm, int code, int reg, int ex, int offset);
extern int vm_CODE_CALL(regvm* vm, int code, int reg, int ex, int offset);

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

regvm::regvm() : reg(), ops{
    vm_CODE_NOP,
    vm_CODE_TRAP,
    vm_CODE_CLEAR,
    vm_CODE_LOAD,
    vm_CODE_STORE,
    vm_CODE_GLOBAL,
    vm_CODE_NEW,
    vm_CODE_BLOCK,
    vm_CODE_CONV,
    vm_CODE_CHG,
    vm_CODE_CMP,
    vm_CODE_JUMP,
    vm_CODE_CALL,
    }
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
        VM_ERROR(ERR_FUNCTION_CALL, start->id, start->reg, start->ex, 0, "Can not get entry function");
        return false;
    }
    core::frame f(this, &r.first->second, 0, 0, 0, 0);
    return f.run();
}

bool regvm::call(core::reg::v& addr, int code, int reg, int ex, int offset)
{
    if (addr.type != TYPE_ADDR)
    {
        return call((int64_t)reg, code, reg, ex, offset);
    }
    else
    {
        core::frame f(*call_stack, call_stack->running, code, reg, ex, offset);
        return f.run((int64_t)reg);
    }
}

bool regvm::call(int64_t id, int code, int reg, int ex, int offset)
{
    auto vm = this;
    auto it = funcs.find(id);
    if (it == funcs.end())
    {
        VM_ERROR(ERR_FUNCTION_CALL, code, reg, ex, offset, "Can not get function info : %lu", id);
        return false;
    }

    core::frame f(*call_stack, &it->second, code, reg, ex, offset);
    return f.run();
}


