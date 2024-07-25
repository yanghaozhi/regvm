#include "frame.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "structs.h"

#include "vm.h"
#include "ext.h"

using namespace core;

frame::frame(frame& cur, func* f, int c, int r, int e, int o) :
    depth(cur.depth + 1), running(f), id(gen_id()), vm(cur.vm), code(c), reg(r), ex(e), offset(o)
{
    up = vm->call_stack;
    down = NULL;
    up->down = this;

    vm->call_stack = this;

    if (vm->vm_call(code, reg, ex, offset, id) == false)
    {
        VM_ERROR(ERR_FUNCTION_CALL, code, reg, ex, offset, "Can not get function info : %lu", id);
        valid = false;
    }
}

frame::frame(regvm* v, func* f, int c, int r, int e, int o) :
    depth(0), running(f), id(gen_id()), vm(v), code(c), reg(r), ex(e), offset(o)
{
    up = NULL;
    down = NULL;
    vm->call_stack = this;
    if (vm->vm_call(code, reg, ex, offset, id) == false)
    {
        VM_ERROR(ERR_FUNCTION_CALL, code, reg, ex, offset, "Can not get function info : %lu", id);
        valid = false;
    }
}

frame::~frame()
{
    if (vm->vm_call(code, reg, ex, offset, -id) == false)
    {
        VM_ERROR(ERR_FUNCTION_CALL, code, reg, ex, offset, "Can not get function info : %lu", id);
        valid = false;
    }

    if (up != NULL)
    {
        up->down = NULL;
    }
    vm->call_stack = up;
}

bool frame::run(int64_t entry)
{
    return (valid == true) ? running->run(vm, entry) : false;
}

int64_t frame::gen_id(void)
{
    int64_t c = running->id;
    c <<= 32;
    c += depth;
    return c;
}
