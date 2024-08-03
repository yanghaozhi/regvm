#include "mem.h"
#include "var.h"

#include "irq.h"
#include "structs.h"

#include "error.h"
#include "scope.h"

#include "../core/vm.h"

using namespace ext;

#define VM static_cast<regvm_mem*>(vm)

bool regvm_mem::vm_init()
{
    return true;
}

bool regvm_mem::vm_exit()
{
    frames.clear();
    return true;
}

core::var* regvm_mem::vm_var(int id)
{
    auto& r = reg.id(id);
    auto v = var::create(r.type, "");
    v->store_from(r);
    return v;
}

core::var* regvm_mem::vm_var(int type, const char* name)
{
    return var::create(type, name); 
}

bool regvm_mem::vm_call(int code, int reg, int ex, int offset, int64_t id)
{
    if ((id > 0) || ((id == 0) && (frames.empty() == true)))
    {
        frames.emplace_front(offset);
    }
    else
    {
        if (frames.front().frame != -id)
        {
            assert(0);
            return false;
        }
        frames.pop_front();
    }
    return true;
}

int regvm_mem::vm_CODE_LOAD(regvm* vm, int code, int reg, int ex, int offset)
{
    auto& e = vm->reg.id(ex);
    auto& r = vm->reg.id(reg);
    auto v = VM->get(e.value.str, false);
    if (v == NULL)
    {
        //auto vm = this;
        //VM_ERROR(ERR_INVALID_VAR, code, reg, ex, offset, "load var name : %s", e.value.str);
        return false;
    }
    else
    {
        return r.load(v);
    }
}

int regvm_mem::vm_CODE_STORE(regvm* vm, int code, int reg, int ex, int offset)
{
    auto& r = vm->reg.id(reg);
    if (ex == reg)
    {
        return r.store();
    }
    else
    {
        auto& e = vm->reg.id(ex);
        if (e.type != TYPE_STRING)
        {
            //auto vm = this;
            //VM_ERROR(ERR_TYPE_MISMATCH, code, reg, ex, offset, "store name : %d", e.type);
            vm->fatal = true;
            return false;
        }
        else
        {
            auto v = VM->get(e.value.str, false);
            if (v != NULL)
            {
                if (v->store_from(r) == true)
                {
                    return true;
                }
            }
            return VM->add(e.value.str, r.type, false)->store_from(r);
        }
    }
    return true;
}

int regvm_mem::vm_CODE_GLOBAL(regvm* vm, int code, int reg, int ex, int offset)
{
    return 0;
}

int regvm_mem::vm_CODE_NEW(regvm* vm, int code, int reg, int ex, int offset)
{
    auto& n = vm->reg.id(reg);
    auto v = VM->get(n.value.str, false);
    if (v != NULL)
    {
        return (v->type == ex) ? true : false;
    }
    v = VM->add(n.value.str, ex, false);
    return (v != NULL) ? true : false;
}

int regvm_mem::vm_CODE_BLOCK(regvm* vm, int code, int reg, int ex, int offset)
{
    if (VM->frames.size() == 0)
    {
        return false;
    }

    switch (ex)
    {
    case 0:
        VM->frames.back().enter_block();
        break;
    case 1:
        VM->frames.back().leave_block();
        break;
    }

    return true;
}

regvm_mem::regvm_mem() : globals(0)
{
    ops[CODE_LOAD]   = vm_CODE_LOAD ;
    ops[CODE_STORE]  = vm_CODE_STORE;
    ops[CODE_GLOBAL] = vm_CODE_GLOBAL;
    ops[CODE_NEW]    = vm_CODE_NEW  ;
    ops[CODE_BLOCK]  = vm_CODE_BLOCK;
}

var* regvm_mem::add(const char* name, const int type, bool global)
{
    auto v = var::create(type, name);
    if (global == false)
    {
        frames.front().scopes.front().add(v);
    }
    else
    {
        globals.add(v);
    }
    return (v->release() == true) ? v : NULL;
}

var* regvm_mem::get(const char* name, bool global) const
{
    const int l = strlen(name);
    uint32_t h = var::calc_hash(name, l);
    if (global == false)
    {
        for (const auto& it : frames.front().scopes)
        {
            auto v = it.get(h, name, l);
            if (v != NULL)
            {
                return v;
            }
        }
    }
    return globals.get(h, name, l);
}

regvm_mem::context::context(int64_t f) : frame(f)
{
    scopes.emplace_front(0);
}

regvm_mem::context::~context()
{
    if (scopes.size() > 1)
    {
        //TODO
        //warning
    }
}

void regvm_mem::context::enter_block()
{
    scopes.emplace_front(scopes.size());
}

void regvm_mem::context::leave_block()
{
    scopes.pop_front();
}

void regvm_mem::dump(regvm* vm, var_cb cb, void* arg, regvm_var_info* info) const
{
    for (auto& f : frames)
    {
        info->func_id = f.frame >> 32;
        info->call_id = f.frame & 0xFFFFFFFF;
        auto it = vm->funcs.find(info->func_id);
        if (it != vm->funcs.end())
        {
            info->func_name = it->second.src.func;
        }
        else
        {
            info->func_name = NULL;
        }

        for (auto& s : f.scopes)
        {
            s.dump(cb, arg, info);
        }
    }
}

#ifdef __cplusplus
extern "C"
{
#endif

bool regvm_debug_var_callback(struct regvm* vm, var_cb cb, void* arg)
{
    regvm_var_info info;
    memset(&info, 0, sizeof(info));
    cb(arg, NULL);

    auto m = static_cast<regvm_mem*>(vm);
    if (m != NULL)
    {
        m->dump(vm, cb, arg, &info);
    }

    cb(arg, (regvm_var_info*)(intptr_t)-1);
    return true;
}


#ifdef __cplusplus
};
#endif


