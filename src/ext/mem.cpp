#include "mem.h"
#include "var.h"

#include "irq.h"
#include "structs.h"

#include "error.h"
#include "scope.h"

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

static bool mem_new(struct regvm* vm, code_t code, int offset, int64_t extra)
{
    auto m = (mem*)vm->ext;
    auto& r = vm->reg.id(code.reg);
    auto v = m->get(r.value.str);
    if (v != NULL)
    {
        return (v->type == code.ex) ? true : false;
    }
    v = m->add(code.ex, r.value.str);
    return (v != NULL) ? true : false;
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
            auto v = m->get(e.value.str);
            if (v != NULL)
            {
                if (v->store(r) == true)
                {
                    return true;
                }
            }
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
    return m->block(extra, code.ex);
}

static bool mem_call(struct regvm* vm, code_t code, int offset, int64_t extra)
{
    auto m = (mem*)vm->ext;
    return m->call(extra);
}


mem::mem() : globals(0)
{
}

mem::~mem()
{
    frames.clear();
}

bool mem::block(int64_t frame, int ex)
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

var* mem::add(const int type, const char* name)
{
    auto v = var::create(type, name);
    frames.front().scopes.front().add(v);
    return (v->release() == true) ? v : NULL;
}

var* mem::get(const char* name) const
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

bool mem::call(int64_t func)
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

mem::context::context(int64_t f) : frame(f)
{
    scopes.emplace_front(0);
}

mem::context::~context()
{
    if (scopes.size() > 1)
    {
        //TODO
        //warning
    }
}

void mem::context::enter_block()
{
    scopes.emplace_front(scopes.size());
}

void mem::context::leave_block()
{
    scopes.pop_front();
}

void mem::dump(regvm* vm, var_cb cb, void* arg, regvm_var_info* info) const
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

struct regvm_ex     var_ext = {mem_init, mem_exit, mem_new, mem_store, mem_load, mem_block, mem_call};

bool regvm_debug_var_callback(struct regvm* vm, var_cb cb, void* arg)
{
    regvm_var_info info;
    memset(&info, 0, sizeof(info));
    cb(arg, NULL);

    auto m = (mem*)vm->ext;
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


