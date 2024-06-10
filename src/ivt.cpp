#include "ivt.h"

#include "vm.h"

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

bool regvm_irq_set(struct regvm* vm, int irq, regvm_irq_handler handler, void* arg)
{
    return vm->idt.set(irq, handler, arg);
}

#ifdef __cplusplus
};
#endif

ivt::ivt()
{
    memset(isrs, 0, sizeof(isrs));
}

bool ivt::set(uint32_t id, regvm_irq_handler func, void* arg)
{
    if (id >= sizeof(isrs) / sizeof(isrs[0])) return false;

    isrs[id].func = func;
    isrs[id].arg = arg;

    return true;
}

int64_t ivt::call(struct regvm* vm, int id, code_t code, int offset, void* args, int64_t defval)
{
    isr& it = isrs[id];
    if (it.func == NULL)
    {
        return defval;
    }
    int64_t r = it.call(vm, id, code, offset, args);
    if ((id == IRQ_ERROR) || (r == 0))
    {
        vm->err.fatal = true;
    }
    return r;
}

int64_t ivt::isr::call(struct regvm* vm, int id, code_t code, int offset, void* extra)
{
    return func(vm, arg, id, code, offset, extra);
}

