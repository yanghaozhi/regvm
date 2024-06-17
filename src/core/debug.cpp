#include <debug.h>

#include <string.h>

#include "vm.h"
#include "error.h"

//TODO
//#include "vm.h"
//#include "error.h"

extern "C"
{

bool regvm_debug_reg_callback(struct regvm* vm, reg_cb cb, void* arg)
{
    regvm_reg_info info;
    memset(&info, 0, sizeof(info));
    cb(arg, NULL);
    core::error::reg_info(vm->reg, [cb, arg](const regvm_reg_info* info)
            {
                cb(arg, info);
            }, &info);
    cb(arg, (regvm_reg_info*)(intptr_t)-1);
    return false;
}

void regvm_debug_uvalue_print(int type, union regvm_uvalue uv)
{
    switch (type)
    {
    case TYPE_SIGNED:
        printf("%lld", (long long)uv.sint);
        break;
    case TYPE_UNSIGNED:
        printf("%llu", (unsigned long long)uv.uint);
        break;
    case TYPE_STRING:
        printf("%s", uv.str);
        break;
    case TYPE_DOUBLE:
        printf("%f", uv.dbl);
        break;
    default:
        break;
    }
}

//bool regvm_debug_var_callback(struct regvm* vm, var_cb cb, void* arg)
//{
//    //regvm_var_info info;
//    //memset(&info, 0, sizeof(info));
//    //cb(arg, NULL);
//    //error::ctx_vars(*vm->ctx, [cb, arg](const regvm_var_info* info)
//    //        {
//    //            cb(arg, info);
//    //        }, &info);
//    //cb(arg, (regvm_var_info*)(intptr_t)-1);
//    return true;
//}

}
