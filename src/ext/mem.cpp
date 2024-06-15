#include "mem.h"
#include "var.h"

#include "irq.h"
#include "structs.h"

#include "../core/vm.h"

using namespace ext;

static bool mem_init(regvm* vm)
{
    auto p = new mem();
    vm->ext = p;
    return true;
}

static bool mem_exit(regvm* vm)
{
    delete (mem*)vm->ext;
    return true;
}

static bool mem_set(struct regvm* vm, code_t code, int offset, int64_t value)
{
    auto& r = vm->reg.id(code.reg);
    //if ((code.ex == TYPE_STRING) && (mem->value & 0x01))
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

static bool mem_store(struct regvm* vm, code_t code, int offset, int64_t extra)
{
    auto& r = vm->reg.id(code.reg);
    if (code.ex == code.reg)
    {
        return r.store();
    }
    else
    {
        auto& e = vm->reg.id(code.ex);
        if ((e.type & 0x07) != TYPE_STRING)
        {
            ERROR(ERR_TYPE_MISMATCH, code, offset, "store name : %d", e.type);
            vm->fatal = true;
            return false;
        }
        else
        {
            auto m = (mem*)vm->ext;
            return m->add(r.type, e.value.str)->store(r);
        }
    }
    return true;
}

static bool mem_load(struct regvm* vm, code_t code, int offset, int64_t extra)
{
    auto m = (mem*)vm->ext;
    auto& e = vm->reg.id(code.ex);
    auto& r = vm->reg.id(code.reg);
    auto v = m->get(e.value.str);
    if (v == NULL)
    {
        ERROR(ERR_INVALID_VAR, code, offset, "load var name : %s", e.value.str);
        return false;
    }
    else
    {
        return v->load(r);
    }
}

static bool mem_block(struct regvm* vm, code_t code, int offset, int64_t extra)
{
    auto m = (mem*)vm->ext;
    switch (code.ex)
    {
    case 0:
        m->enter_block();
        break;
    case 1:
        m->leave_block();
        break;
    }
    return true;
}

static bool mem_call(struct regvm* vm, code_t code, int offset, int64_t extra)
{
    return true;
}

struct regvm_ex     var_ext = {mem_init, mem_exit, mem_set, mem_store, mem_load, mem_block, mem_call};


mem::mem() : globals(0)
{
}

mem::~mem()
{
}

void mem::enter_block()
{
    scopes.emplace_front(scopes.size() + 1);
}

void mem::leave_block()
{
    scopes.pop_front();
}

var* mem::add(const int type, const char* name)
{
    auto v = var::create(type, name);
    scopes.front().add(v);
    return (v->release() == true) ? v : NULL;
}

var* mem::get(const char* name) const
{
    const int l = strlen(name);
    uint32_t h = var::calc_hash(name, l);
    for (const auto& it : scopes)
    {
        auto v = it.get(h, name, l);
        if (v != NULL)
        {
            return v;
        }
    }
    return globals.get(h, name, l);
}

