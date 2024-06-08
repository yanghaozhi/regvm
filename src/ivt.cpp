#include "ivt.h"

#include "vm.h"

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

bool regvm_irq_set(struct regvm* vm, int irq, void* handler)
{
    return vm->idt.set(irq, handler, NULL);
}

#ifdef __cplusplus
};
#endif

ivt::ivt()
{
    memset(isrs, 0, sizeof(isrs));
}

bool ivt::set(uint32_t id, void* func, void* arg)
{
    if (id >= sizeof(isrs) / sizeof(isrs[0])) return false;

    isrs[id].func = func;
    isrs[id].arg = arg;

    return true;
}

