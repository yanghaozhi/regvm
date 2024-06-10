#include "ivt.h"

#include "vm.h"

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

bool regvm_irq_set(struct regvm* vm, int irq, regvm_irq_handler handler)
{
    return vm->idt.set(irq, handler);
}

#ifdef __cplusplus
};
#endif

ivt::ivt()
{
    memset(isrs, 0, sizeof(isrs));
}

bool ivt::set(uint32_t id, regvm_irq_handler func)
{
    if (id >= sizeof(isrs) / sizeof(isrs[0])) return false;

    isrs[id].func = func;

    return true;
}

int ivt::call(struct regvm* vm, int id, code_t code, int offset, void* args)
{
    if (isrs[id].func == NULL)
    {
        return 0;
    }
    int r = isrs[id].func(vm, id, code, offset, args);
    if ((id == IRQ_ERROR) || (r == 0))
    {
        vm->err.fatal = true;
    }
    return r;
}
