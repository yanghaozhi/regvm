#include "vm.h"

#include <code.h>

#include "ext.h"

#include <vector>


struct regvm_ext
{
    void*   arg;
    bool (*init)(regvm* vm, int idx, void* arg);
    bool (*exit)(regvm* vm, int idx, void* arg);
};
std::vector<regvm_ext>  exts;

vm_op_t                 vm_code_ops[256 - CODE_TRAP]    = {NULL};
regvm_ext_op            vm_ext_ops;
//vm_op_t*     vm_ops = vm_ops_impl;

extern "C"
{

void vm_add_ext(void* arg, vm_ext_t init, vm_ext_t exit)
{
    exts.push_back({arg, init, exit});
}

struct regvm* regvm_init(int ext_inits, ...)
{
    va_list ap;
    va_start(ap, ext_inits);
    for (int i = 0; i < ext_inits; i++)
    {
        auto f = va_arg(ap, regvm_ext_init);
        int r = f();
        if (r != 0)
        {
            LOGE("extension %d init FAILED : %d", i, r);
            return NULL;
        }
    }
    va_end(ap);

    //auto vm = new REGVM_IMPL();
    //CRTP_CALL(vm_init);
    const int s = sizeof(regvm) + sizeof(void*) * exts.size();
    char* p = (char*)malloc(s);
    memset(p, 0, s);
    auto vm = new (p) regvm();

    int i = 0;
    for (auto& it : exts)
    {
        it.init(vm, i++, it.arg);
    }

    return vm;
}

bool regvm_exit(struct regvm* vm)
{
    //CRTP_CALL(vm_exit);
    //delete vm;
    int i = 0;
    for (auto& it : exts)
    {
        it.exit(vm, i++, it.arg);
    }

    vm->~regvm();
    free(vm);

    return true;
}

}

regvm::regvm() : reg()
{
#ifdef DEBUG
    memset(code_names, 0, sizeof(code_names));
#define SET_NAME(x) code_names[CODE_##x] = #x;
#else
#define SET_NAME(x)
#endif

#define SET_1(name)                                                 \
    extern int vm_CODE_##name(regvm*, code_t, int, const void*);    \
    if (vm_code_ops[CODE_##name - CODE_TRAP] == NULL)               \
    {                                                               \
        vm_code_ops[CODE_##name - CODE_TRAP] = vm_CODE_##name;      \
    }
#define SET_0(name)
#define SET_OPS(func, name)     MLIB_CAT(SET_, func)(name); SET_NAME(name);

    SET_OPS(0, NOP);
    SET_OPS(0, DATA);
    SET_OPS(0, MOVE);
    SET_OPS(0, CHG);
    SET_OPS(0, CMP);
    SET_OPS(0, TYPE);
    SET_OPS(0, CALC);
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
    SET_OPS(0, JUMP);
    SET_OPS(0, JCMP);
    SET_OPS(0, JEQ);
    SET_OPS(0, JNE);
    SET_OPS(0, JGT);
    SET_OPS(0, JGE);
    SET_OPS(0, JLT);
    SET_OPS(0, JLE);
    SET_OPS(0, SET);

    SET_OPS(1, TRAP);
    SET_OPS(1, LOAD);
    SET_OPS(1, CLEAR);
    SET_OPS(1, STORE);
    SET_OPS(1, BLOCK);
    SET_OPS(1, CALL);
    SET_OPS(1, RET);

    SET_OPS(1, ECHO);

    SET_OPS(1, SLEN);

    SET_OPS(1, LLEN);
    SET_OPS(1, LAT);
    SET_OPS(1, LSET);
    SET_OPS(1, LPUSH);
    SET_OPS(1, LPOP);
    SET_OPS(1, LINSERT);
    SET_OPS(1, LERASE);

    SET_OPS(1, DLEN);
    SET_OPS(1, DGET);
    SET_OPS(1, DSET);
    SET_OPS(1, DDEL);
    SET_OPS(1, DHAS);
    SET_OPS(1, DITEMS);

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
    core::func f(start, count, 0, &src, VM_CODE_SHARE);
    core::frame entry(this, &f, *start, 0);

    reg.pages[0] = &root;
    reg.pages[1] = &entry.sub_func;

    //root.prepare();
    //entry.sub_func.prepare();

    bool r = entry.run();

    root.cleanup();

    return r;
}

//bool regvm::call(core::reg::v& addr, code_t code, int offset)
//{
//    if (addr.type != TYPE_ADDR)
//    {
//        return call((int64_t)addr, code, offset);
//    }
//    else
//    {
//        core::frame f(*call_stack, call_stack->running, code, offset);
//        return f.run((int64_t)addr);
//    }
//}

bool regvm::call(int32_t id, code_t code, int offset)
{
    auto vm = this;
    auto it = funcs.find(id);
    if (it == funcs.end())
    {
        VM_ERROR(ERR_FUNCTION_CALL, code, offset, "Can not get function info : %d", id);
        return false;
    }

    core::frame sub(*call_stack, &it->second, code, offset);

    auto* p= reg.pages[0];
    reg.pages[0] = reg.pages[1];
    reg.pages[1] = &sub.sub_func;

    //sub.sub_func.prepare();
    LOGT("pages : %p - %p", reg.pages[0], reg.pages[1]);

    bool r = sub.run();

    sub.sub_func.cleanup();

    reg.pages[1] = reg.pages[0];
    reg.pages[0] = p;

    return r;
}


