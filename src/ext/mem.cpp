#include "mem.h"

#include "irq.h"
#include "structs.h"

//bool vm_set(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
//{
//    regvm_mem* mem = (regvm_mem*)extra;
//    core::regv* r = (core::regv*)mem->reg;
//    if ((code.ex == TYPE_STRING) && (mem->value & 0x01))
//    {
//    //    auto& it = vm->idt.isrs[IRQ_STR_RELOCATE];
//    //    if (it.func == NULL)
//    //    {
//    //        ERROR(ERR_STRING_RELOCATE, code, offset, "need to relocate string : %ld", value);
//    //        return false;
//    //    }
//    //    value = it.call(vm, IRQ_STR_RELOCATE, code, offset, (void*)value);
//    //    if (value == 0)
//    //    {
//    //        ERROR(ERR_STRING_RELOCATE, code, offset, "relocate string : %ld ERROR", value);
//    //        return false;
//    //    }
//    }
//    return r->set(value, code.ex);
//}

bool vm_store(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
{
    regvm_mem* mem = (regvm_mem*)extra;
    core::regv* r = (core::regv*)mem->reg;
    if (code.ex == 0)
    {
        return r->store();
    }
    else
    {
        core::regv* e = (core::regv*)mem->ex;
        if ((e->type & 0x07) != TYPE_STRING)
        {
            //TODO
            //ERROR(ERR_TYPE_MISMATCH, code, offset, "store name : %d", e->type);
            //vm->fatal = true;
            return false;
        }
        else
        {
            //TODO
            //var* v = vm->ctx->add(r.type, e.value.str);
            //return r->store(v);
        }
    }
    return true;
}

bool vm_load(struct regvm* vm, const code_t code)
{
    //TODO
    //auto& e = vm->reg.id(code.ex);
    //auto& r = vm->reg.id(code.reg);
    //return r.load(vm->ctx->get(e.value.str));
    return true;
}

