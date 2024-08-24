#include "mem.h"
#include "var.h"

#include "irq.h"
#include "structs.h"

#include "log.h"
#include "error.h"

#include "../core/vm.h"


#define VM static_cast<regvm_mem*>(vm)

using namespace ext;

bool regvm_mem::vm_init()
{
    return true;
}

bool regvm_mem::vm_exit()
{
    //frames.clear();
    return true;
}

core::var* regvm_mem::vm_var(int id)
{
    auto& r = reg.id(id);
    auto v = new var(r.type, (uint64_t)r);
    v->store_from(r);
    return v;
}

core::var* regvm_mem::vm_var(int type, uint64_t id)
{
    return new var(type, id); 
}

bool regvm_mem::vm_call(code_t code, int offset, int64_t id)
{
    if (unlikely(id == 0))
    {
        cur_call = 0;
        return true;
    }

    if (id > 0)
    {
        cur_call = id;
        calls.push_back(id);
    }
    else
    {
        id = -id;
        if (unlikely(id != calls.back()))
        {
            LOGE("exist call %lld, but it's NOT match %lld", (long long)id, (long long)calls.back());
            return false;
        }
        calls.pop_back();
        cur_call = calls.back();

        auto first = vars.lower_bound(id);
        auto last = first;
        while ((last != vars.end()) && (last->first & 0xFFFFFFFFFFFF0000) == (uint64_t)id)
        {
            last->second->release();
            ++last;
        }
        vars.erase(first, last);
    }

    return true;
}

int regvm_mem::vm_CODE_LOAD(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& e = vm->reg.id(code.b);
    auto& r = vm->reg.id(code.a);
    auto v = VM->get(e);
    if (v == NULL)
    {
        //auto vm = this;
        //VM_ERROR(ERR_INVALID_VAR, code, reg, ex, offset, "load var name : %s", e.value.str);
        return 0;
    }
    else
    {
        return r.load(v);
    }
}

int regvm_mem::vm_CODE_STORE(regvm* vm, code_t code, int offset, const void* extra)
{
    auto& r = vm->reg.id(code.a);
    switch (code.c)
    {
    case 0:     //原始加载的变量（如无，则无操作）
        return r.store();
    case 1:
    case 2:
        {
            auto& e = vm->reg.id(code.b);
            switch (e.type)
            {
            case TYPE_STRING:
                return 0;
            case TYPE_ADDR:
                {
                    auto vid = VM->var_id(e.value.uint);
                    auto v = VM->get(vid);
                    if (v != NULL)
                    {
                        if (v->store_from(r) == true)
                        {
                            return 1;
                        }
                    }
                    return VM->add(vid, r.type)->store_from(r);
                }
            default:
                //auto vm = this;
                //VM_ERROR(ERR_TYPE_MISMATCH, code, reg, ex, offset, "store name : %d", e.type);
                vm->fatal = true;
                return 0;
            }
        }
        break;
    case 3:     //写入到新的变量中（不查询上级scope的同名变量，如本级scope找不到则新建）
        {
            auto& n = vm->reg.id(code.b);
            auto vid = VM->var_id(n.value.uint);
            auto v = VM->get(vid);
            if (v != NULL)
            {
                return (v->type == code.a) ? true : false;
            }
            v = VM->add(vid, code.a);
            return (v != NULL) ? 1 : 0;
        }
    case 4:     //只在顶层scope（全局变量）中查找或新建
        break;
    case 5:
        {
            auto& e = vm->reg.id(code.b);
            auto vid = VM->var_id(e.value.uint);
            switch (e.type)
            {
            case TYPE_STRING:
                return 0;
            case TYPE_ADDR:
                return VM->del(vid);
            default:
                //auto vm = this;
                //VM_ERROR(ERR_TYPE_MISMATCH, code, reg, ex, offset, "store name : %d", e.type);
                vm->fatal = true;
                return 0;
            }
        }
        break;
    default:
        return 0;
    }
    return 1;
}

int regvm_mem::vm_CODE_BLOCK(regvm* vm, code_t code, int offset, const void* extra)
{
    //if (VM->frames.size() == 0)
    //{
    //    return false;
    //}

    //switch (code.b)
    //{
    //case 0:
    //    VM->frames.back().enter_block();
    //    break;
    //case 1:
    //    VM->frames.back().leave_block();
    //    break;
    //}

    return 1;
}

struct mem_init
{
    mem_init()
    {
#define REG_OP(x)   vm_ops[CODE_##x - CODE_TRAP] = regvm_mem::vm_CODE_##x;
        REG_OP(LOAD);
        REG_OP(STORE);
        //REG_OP(GLOBAL);
        //REG_OP(NEW);
        //REG_OP(BLOCK);
#undef REG_OP
    }
};

static mem_init     s_init;

regvm_mem::regvm_mem()
{
}

var* regvm_mem::add(uint64_t id, const int type)
{
    auto v = new var(type, id);

    auto r = vars.emplace(id, v);
    if (r.second == false)
    {
        v->release();
        return NULL;
    }
    return v;

    //if (global == false)
    //{
    //    frames.front().scopes.front().add(v);
    //}
    //else
    //{
    //    globals.add(v);
    //}
    //return (v->release() == true) ? v : NULL;
}

var* regvm_mem::get(uint64_t id) const
{
    auto it = vars.find(id);
    return (it != vars.end()) ? it->second : NULL;
}

bool regvm_mem::del(uint64_t id)
{
    auto it = vars.find(id);
    if (likely(it != vars.end()))
    {
        it->second->release();
        vars.erase(it);
        return true;
    }
    return false;
}

//regvm_mem::context::context(int64_t f) : frame(f)
//{
//    scopes.emplace_front(0);
//}
//
//regvm_mem::context::~context()
//{
//    if (scopes.size() > 1)
//    {
//        //TODO
//        //warning
//    }
//}
//
//void regvm_mem::context::enter_block()
//{
//    scopes.emplace_front(scopes.size());
//}
//
//void regvm_mem::context::leave_block()
//{
//    scopes.pop_front();
//}

void regvm_mem::dump(regvm* vm, var_cb cb, void* arg, regvm_var_info* info) const
{
    //for (auto& f : frames)
    //{
    //    info->func_id = f.frame >> 32;
    //    info->call_id = f.frame & 0xFFFFFFFF;
    //    auto it = vm->funcs.find(info->func_id);
    //    if (it != vm->funcs.end())
    //    {
    //        info->func_name = it->second.src.func;
    //    }
    //    else
    //    {
    //        info->func_name = NULL;
    //    }

    //    for (auto& s : f.scopes)
    //    {
    //        s.dump(cb, arg, info);
    //    }
    //}
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


