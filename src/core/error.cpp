#include "error.h"

#include "vm.h"

using namespace core;

error::error(void)
{
    code = ERR_OK;

    memset(&src, 0, sizeof(src));
    memset(&func, 0, sizeof(func));
#ifdef DEBUG
    memset(&self, 0, sizeof(self));
#endif
    reason[0] = '\0';
}

void error::set(regvm* vm, int errcode, int code, int reg, int ex, int offset, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(reason, sizeof(reason), fmt, ap);
    va_end(ap);

    code = errcode;
    reason[sizeof(reason) - 1] = '\0';
    if (vm->call_stack->running != NULL)
    {
        func = vm->call_stack->running->src;
    }

    regvm_error err;
    memset(&err, 0, sizeof(err));
    if (vm->call_stack->cur != NULL)
    {
        src = *vm->call_stack->cur;
        err.src = &src;
    }
    err.code = errcode;
    err.reason = reason;
#ifdef DEBUG
    err.self = &vm->err.self;
#endif
    vm->idt.call(vm, IRQ_ERROR, code, reg, ex, offset, &err);
}

