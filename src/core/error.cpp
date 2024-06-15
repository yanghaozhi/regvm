#include "error.h"

#include "vm.h"


void error::set(struct regvm* vm, int errcode, code_t cur, int offset, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(reason, sizeof(reason), fmt, ap);
    va_end(ap);

    code = errcode;
    reason[sizeof(reason) - 1] = '\0';
    func = vm->call_stack->running->src;

    regvm_error err;
    if (vm->call_stack->cur != NULL)
    {
        src = *vm->call_stack->cur;
        err.src = &src;
    }
    err.code = errcode;
    err.reason = reason;
    vm->idt.call(vm, IRQ_ERROR, cur, offset, &err);
}
