#include <debug.h>

#include "vm.h"

class debug : public error
{
public:
    static bool reg_cb(struct regvm* vm, reg_cb cb, void* arg)
    {
        const int size = 16;
        for (int i = 1; i < size; i++)
        {
            cb(i, vm->reg.values[i].num, vm->reg.froms[i], vm->reg.types[i], arg);
            //reg_info(vm, i, [i, arg](int64_t value, void* from, int type)
            //        {
            //            cb(i, value, var, type, arg);
            //        });
        }
        return true;
    }

    static bool var_cb(struct regvm* vm, var_cb cb, void* arg)
    {
        vm->globals.for_each([cb, arg](var* v)
                {
                    if (v != NULL)
                    {
                        cb(0, v->name, v->value.num, v->type, v->reg, v->ref, arg);
                    }
                });
        int id = 1;
        for (auto it = vm->ctx->scopes.rbegin(); it != vm->ctx->scopes.rend(); ++it)
        {
            it->for_each([id, cb, arg](var* v)
                {
                    if (v != NULL)
                    {
                        cb(id, v->name, v->value.num, v->type, v->reg, v->ref, arg);
                    }
                });
            ++id;
        }

        return true;
    }
};

extern "C"
{

bool regvm_debug_reg_callback(struct regvm* vm, reg_cb cb, void* arg)
{
    return debug::reg_cb(vm, cb, arg);
}

bool regvm_debug_var_callback(struct regvm* vm, var_cb cb, void* arg)
{
    return debug::var_cb(vm, cb, arg);
}

}
