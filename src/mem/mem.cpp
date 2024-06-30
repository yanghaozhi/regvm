#include "mem.h"
#include "var.h"

#include "irq.h"
#include "structs.h"

#include "error.h"
#include "scope.h"

#include "../core/vm.h"

using namespace ext;

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
    v->store(r);
    return v;
}

bool regvm_mem::vm_new(code_t code, int offset, int64_t extra)
{
    auto& n = reg.id(code.reg);
    auto v = get(n.value.str);
    if (v != NULL)
    {
        return (v->type == code.ex) ? true : false;
    }
    v = add(code.ex, n.value.str);
    return (v != NULL) ? true : false;
}

bool regvm_mem::vm_store(code_t code, int offset, int64_t extra)
{
    auto& r = reg.id(code.reg);
    if (code.ex == code.reg)
    {
        return r.store();
    }
    else
    {
        auto& e = reg.id(code.ex);
        if ((e.type & 0x07) != TYPE_STRING)
        {
            auto vm = this;
            VM_ERROR(ERR_TYPE_MISMATCH, code, offset, "store name : %d", e.type);
            fatal = true;
            return false;
        }
        else
        {
            auto v = get(e.value.str);
            if (v != NULL)
            {
                if (v->store(r) == true)
                {
                    return true;
                }
            }
            return add(r.type, e.value.str)->store(r);
        }
    }
    return true;
}

bool regvm_mem::vm_load(code_t code, int offset, int64_t extra)
{
    auto& e = reg.id(code.ex);
    auto& r = reg.id(code.reg);
    auto v = get(e.value.str);
    if (v == NULL)
    {
        auto vm = this;
        VM_ERROR(ERR_INVALID_VAR, code, offset, "load var name : %s", e.value.str);
        return false;
    }
    else
    {
        return v->load(r);
    }
}

bool regvm_mem::vm_block(code_t code, int offset, int64_t extra)
{
    return block(extra, code.ex);
}

bool regvm_mem::vm_call(code_t code, int offset, int64_t extra)
{
    return call(extra);
}

regvm_mem::regvm_mem() : globals(0)
{
}

bool regvm_mem::block(int64_t frame, int ex)
{
    if (frames.size() == 0)
    {
        return false;
    }

    switch (ex)
    {
    case 0:
        frames.back().enter_block();
        break;
    case 1:
        frames.back().leave_block();
        break;
    }

    return true;
}

var* regvm_mem::add(const int type, const char* name)
{
    auto v = var::create(type, name);
    frames.front().scopes.front().add(v);
    return (v->release() == true) ? v : NULL;
}

var* regvm_mem::get(const char* name) const
{
    const int l = strlen(name);
    uint32_t h = var::calc_hash(name, l);
    for (const auto& it : frames.front().scopes)
    {
        auto v = it.get(h, name, l);
        if (v != NULL)
        {
            return v;
        }
    }
    return globals.get(h, name, l);
}

bool regvm_mem::call(int64_t func)
{
    if ((func > 0) || ((func == 0) && (frames.empty() == true)))
    {
        frames.emplace_front(func);
    }
    else
    {
        if (frames.front().frame != -func)
        {
            assert(0);
            return false;
        }
        frames.pop_front();
    }
    return true;
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


