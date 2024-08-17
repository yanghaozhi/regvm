#include "frame.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "structs.h"

#include "vm.h"
#include "ext.h"

using namespace core;

frame::frame(frame& cur, func* f, code_t c, int o) :
    depth(cur.depth + 1), running(f), id(gen_id()), vm(cur.vm), code(c), offset(o)
{
    if (unlikely(depth <= cur.depth))
    {
        VM_ERROR(ERR_FUNCTION_CALL, c, offset, "stack is OVERFLOWED !!! : %d", cur.depth);
        valid = false;
    }

    up = vm->call_stack;
    down = NULL;
    up->down = this;

    vm->call_stack = this;

    if ((unlikely(valid == false)) || (unlikely(vm->vm_call(code, offset, id) == false)))
    {
        VM_ERROR(ERR_FUNCTION_CALL, c, offset, "Can not get function info : %lu", id);
        valid = false;
    }
}

frame::frame(regvm* v, func* f, code_t c, int o) :
    depth(0), running(f), id(gen_id()), vm(v), code(c), offset(o)
{
    up = NULL;
    down = NULL;
    vm->call_stack = this;
    if (unlikely(vm->vm_call(code, offset, id) == false))
    {
        VM_ERROR(ERR_FUNCTION_CALL, code, offset, "Can not get function info : %lu", id);
        valid = false;
    }
}

frame::~frame()
{
    if (unlikely(vm->vm_call(code, offset, -id) == false))
    {
        VM_ERROR(ERR_FUNCTION_CALL, code, offset, "Can not get function info : %lu", id);
        valid = false;
    }

    if (up != NULL)
    {
        up->down = NULL;
    }
    vm->call_stack = up;
}

int frame::run(void)
{
    return (valid == true) ? running->run(vm) : ERROR;
}

int64_t frame::gen_id(void)
{
    int64_t c = running->id;
    c <<= 32;
    c += depth;
    return c;
}
