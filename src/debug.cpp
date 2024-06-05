#include <debug.h>

#include "vm.h"
#include "error.h"

extern "C"
{

bool regvm_debug_reg_callback(struct regvm* vm, reg_cb cb, void* arg)
{
    vm->err.reg_info(vm->reg, [cb, arg](int id, int64_t value, void* from, int type)
            {
                cb(arg, id, value, from, type);
            });
    return true;
}

bool regvm_debug_var_callback(struct regvm* vm, var_cb cb, void* arg)
{
    vm->err.ctx_vars(*vm->ctx, [cb, arg](int id, var* v, int ref)
            {
                cb(arg, id, v->name, v->value.num, v->type, v->reg, ref);
            });
    return true;
}

}
