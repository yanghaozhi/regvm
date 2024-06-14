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


//遇到TRAP（调试）指令时发起
//返回值为继续n条指令后再次自动发起，0表示不再发起
static int64_t irq_TRAP(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
{
    return 1;
}

//发生内部错误时发起
static int64_t irq_ERROR(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
{
    uint32_t c = *(uint16_t*)&code;
    printf("\e[31m%X @ %d - %p \e[0m\n", c, offset, extra);

    if (extra != NULL)
    {
        regvm_error* e = (regvm_error*)extra;
        if (e->src != NULL)
        {
            printf("\e[31m\t %d @ %s:%d:%s - %s \e[0m\n", e->code, e->src->file, e->src->line, e->src->func, e->reason);
        }
        else
        {
            printf("\e[31m\t %d @ %p - %s \e[0m\n", e->code, e->src, e->reason);
        }
    }
    return 0;
}


//需要获取当前源码的文件名/行号/函数名
static int64_t irq_LOCATION(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
{
    return 0;
}

////重定位字符串地址
////当调用SET指令时，如果类型为STRING，且指针值为奇数（合法指针值不会为奇数）
////则会发起该中断以获得具体的真实地址
//static int64_t irq_STR_RELOCATE(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
//{
//    return 0;
//}

    //发起函数调用
    //需要在此中断中提供新函数的具体信息
static int64_t irq_FUNCTION_CALL(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
{
    return 0;
}

ivt::ivt()
{
    memset(isrs, 0, sizeof(isrs));

#define SET_DEFAULT(x, e)               \
    {                                   \
        isr& it = isrs[IRQ_##x];        \
        it.func = irq_##x;              \
        it.def_func = irq_##x;          \
        it.id = IRQ_##x;                \
        it.check_err = e;               \
    }
    SET_DEFAULT(TRAP, isr::DO_NOT_CHECK);
    SET_DEFAULT(ERROR, 0);
    SET_DEFAULT(LOCATION, -1);
    //SET_DEFAULT(STR_RELOCATE, 0);
    SET_DEFAULT(FUNCTION_CALL, 0);
#undef SET_DEFAULT
}

bool ivt::set(uint32_t id, regvm_irq_handler func, void* arg)
{
    if (id >= sizeof(isrs) / sizeof(isrs[0])) return false;

    if (func != NULL)
    {
        isrs[id].func = func;
        isrs[id].arg = arg;
    }
    else
    {
        isrs[id].func = isrs[id].def_func;
        isrs[id].arg = isrs[id].arg;
    }

    return true;
}

int64_t ivt::call(struct regvm* vm, int id, code_t code, int offset, void* args, int64_t defval)
{
    isr& it = isrs[id];
    int64_t r = it.call(vm, id, code, offset, args);
    if ((id == IRQ_ERROR) || ((r == it.check_err) && (it.check_err != isr::DO_NOT_CHECK)))
    {
        vm->fatal = true;
    }
    return r;
}

int64_t ivt::isr::call(struct regvm* vm, int id, code_t code, int offset, void* extra)
{
    return func(vm, arg, code, offset, extra);
}

