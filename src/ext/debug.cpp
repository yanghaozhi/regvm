#include <debug.h>

//TODO
//#include "vm.h"
//#include "error.h"

extern "C"
{

bool regvm_debug_reg_callback(struct regvm* vm, reg_cb cb, void* arg)
{
    //regvm_reg_info info;
    //memset(&info, 0, sizeof(info));
    //cb(arg, NULL);
    //error::reg_info(vm->reg, [cb, arg](const regvm_reg_info* info)
    //        {
    //            cb(arg, info);
    //        }, &info);
    //cb(arg, (regvm_reg_info*)(intptr_t)-1);
    return true;
}

bool regvm_debug_var_callback(struct regvm* vm, var_cb cb, void* arg)
{
    //regvm_var_info info;
    //memset(&info, 0, sizeof(info));
    //cb(arg, NULL);
    //error::ctx_vars(*vm->ctx, [cb, arg](const regvm_var_info* info)
    //        {
    //            cb(arg, info);
    //        }, &info);
    //cb(arg, (regvm_var_info*)(intptr_t)-1);
    return true;
}

}