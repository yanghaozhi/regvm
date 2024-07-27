#include "vm.h"

#include <code.h>

#include "ext.h"


extern int vm_CODE_NOP(regvm* vm, code_t code, int offset, const void* extra);
extern int vm_CODE_SET(regvm* vm, code_t code, int offset, const void* extra);
extern int vm_CODE_TRAP(regvm* vm, code_t code, int offset, const void* extra);
extern int vm_CODE_CLEAR(regvm* vm, code_t code, int offset, const void* extra);
extern int vm_CODE_LOAD(regvm* vm, code_t code, int offset, const void* extra);
extern int vm_CODE_STORE(regvm* vm, code_t code, int offset, const void* extra);
extern int vm_CODE_BLOCK(regvm* vm, code_t code, int offset, const void* extra);
extern int vm_CODE_CONV(regvm* vm, code_t code, int offset, const void* extra);
extern int vm_CODE_CALL(regvm* vm, code_t code, int offset, const void* extra);
extern int vm_CODE_RET(regvm* vm, code_t code, int offset, const void* extra);
extern int vm_CODE_ECHO(regvm* vm, code_t code, int offset, const void* extra);

extern "C"
{

struct regvm* regvm_init(void)
{
    auto vm = new REGVM_IMPL();
    CRTP_CALL(vm_init);
    return vm;
}

bool regvm_exit(struct regvm* vm)
{
    CRTP_CALL(vm_exit);
    delete vm;
    return true;
}

}

regvm::regvm() : reg(), ops{NULL}
{
#ifdef DEBUG
    memset(code_names, 0, sizeof(code_names));
#define SET_NAME(x) code_names[CODE_##x] = #x;
#else
#define SET_NAME(x)
#endif

#define SET_1(name)             ops[CODE_##name - CODE_TRAP] = vm_CODE_##name;
#define SET_0(name)
#define SET_OPS(func, name)     MLIB_CAT(SET_, func)(name); SET_NAME(name);

    SET_OPS(0, NOP);
    SET_OPS(0, DATA);
    SET_OPS(0, MOVE);
    SET_OPS(0, CHG);
    SET_OPS(0, CMP);
    SET_OPS(0, TYPE);
    SET_OPS(0, INC);
    SET_OPS(0, ADD);
    SET_OPS(0, SUB);
    SET_OPS(0, MUL);
    SET_OPS(0, DIV);
    SET_OPS(0, MOD);
    SET_OPS(0, AND);
    SET_OPS(0, OR);
    SET_OPS(0, XOR);
    SET_OPS(0, SHL);
    SET_OPS(0, SHR);
    SET_OPS(0, SHIFT);
    SET_OPS(0, JUMP);
    SET_OPS(0, JEQ);
    SET_OPS(0, JNE);
    SET_OPS(0, JGT);
    SET_OPS(0, JGE);
    SET_OPS(0, JLT);
    SET_OPS(0, JLE);

    SET_OPS(1, TRAP);
    SET_OPS(1, SET);
    SET_OPS(1, LOAD);
    SET_OPS(1, CLEAR);
    SET_OPS(1, STORE);
    SET_OPS(1, BLOCK);
    SET_OPS(1, CONV);
    SET_OPS(1, CALL);
    SET_OPS(1, RET);

    SET_OPS(1, ECHO);

    SET_OPS(0, SLEN);

    SET_OPS(0, LLEN);
    SET_OPS(0, LAT);
    SET_OPS(0, LSET);
    SET_OPS(0, LPUSH);
    SET_OPS(0, LPOP);
    SET_OPS(0, LERASE);

    SET_OPS(0, DLEN);
    SET_OPS(0, DGET);
    SET_OPS(0, DSET);
    SET_OPS(0, DDEL);
    SET_OPS(0, DHAS);
    SET_OPS(0, DITEMS);

    SET_OPS(0, EXIT);

#undef SET_OPS
#undef SET_NAME
}

regvm::~regvm()
{
    while (call_stack != NULL)
    {
        auto p = call_stack;
        call_stack = call_stack->down;
        delete p;
    }
}

bool regvm::run(const code_t* start, int count)
{
    regvm_src_location src = {0, "NULL", "..."};
    auto r = funcs.try_emplace((int32_t)0, start, count, 0, count, 0, &src);
    if (r.second == false)
    {
        auto vm = this;
        VM_ERROR(ERR_FUNCTION_CALL, *start, 0, "Can not get entry function");
        return false;
    }
    core::frame f(this, &r.first->second, *start, 0);
    return f.run();
}

bool regvm::call(core::reg::v& addr, code_t code, int offset)
{
    if (addr.type != TYPE_ADDR)
    {
        return call((int64_t)addr, code, offset);
    }
    else
    {
        core::frame f(*call_stack, call_stack->running, code, offset);
        return f.run((int64_t)addr);
    }
}

bool regvm::call(int64_t id, code_t code, int offset)
{
    auto vm = this;
    auto it = funcs.find(id);
    if (it == funcs.end())
    {
        VM_ERROR(ERR_FUNCTION_CALL, code, offset, "Can not get function info : %lu", id);
        return false;
    }

    core::frame f(*call_stack, &it->second, code, offset);
    return f.run();
}


