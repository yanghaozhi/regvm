#include <debug.h>

#include "vm.h"

class debug
{
public:
    static bool reg_info(struct regvm* vm, int id, uint64_t* value, void** vars, int* type)
    {
        if (regs::valid_id(id) == false) return false;

        *value = vm->reg.values[id].num;
        *vars = vm->reg.froms[id];
        *type = vm->reg.types[id];

        return true;
    }
};

extern "C"
{

bool regvm_debug_reg_info(struct regvm* vm, int id, uint64_t* value, void** vars, int* type)
{
    //vm->reg.set(inst->reg, inst->value.num, inst->type);
    return debug::reg_info(vm, id, value, vars, type);
}

}
