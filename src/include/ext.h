#pragma once

#include <stdlib.h>
#include <string.h>

#include "mlib.h"
#include "ext_type.h"

#include "vm.h"

#include <map>


#define CRTP_CALL(name, ...)    static_cast<regvm_crtp<REGVM_IMPL>*>(vm)->name(__VA_ARGS__)


template <typename T> struct regvm_crtp : public regvm
{
#define CRTP_FUNC(name, ret, argc, ...)                                             \
    inline ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__))             \
    {                                                                               \
        return static_cast<T*>(this)->name(MLIB_CALL_LIST(argc, __VA_ARGS__));      \
    }

    CRTP_FUNC(vm_init,  bool, 0);
    CRTP_FUNC(vm_exit,  bool, 0);
    CRTP_FUNC(vm_var,   core::var*, 1, int);
    CRTP_FUNC(vm_var,   core::var*, 2, int, const char*);   //type, name

    CRTP_FUNC(vm_new,   bool, 5, int, int, int, int, int64_t);
    CRTP_FUNC(vm_store, bool, 5, int, int, int, int, int64_t);
    CRTP_FUNC(vm_load,  bool, 5, int, int, int, int, int64_t);
    CRTP_FUNC(vm_block, bool, 5, int, int, int, int, int64_t);
    CRTP_FUNC(vm_call,  bool, 5, int, int, int, int, int64_t);

#undef CRTP_FUNC
};


#ifdef REGVM_EXT

#include "regvm_ext.h"

#else
namespace ext
{
struct var : public core::var_crtp<var>
{
    const uint16_t      type;
    bool release(void) const    {return false;};
};
}

struct regvm_core : public regvm_crtp<regvm_core>
{
    typedef ext::var     var_t;
    bool release(void) const    {return false;};

#define CRTP_FUNC(name, ret, argc, ...)                                             \
    ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__));

    CRTP_FUNC(vm_init,  bool, 0);
    CRTP_FUNC(vm_exit,  bool, 0);
    CRTP_FUNC(vm_var,   core::var*, 1, int);
    CRTP_FUNC(vm_var,   core::var*, 2, int, const char*);   //type, name

    CRTP_FUNC(vm_new,   bool, 5, int, int, int, int, int64_t);
    CRTP_FUNC(vm_store, bool, 5, int, int, int, int, int64_t);
    CRTP_FUNC(vm_load,  bool, 5, int, int, int, int, int64_t);
    CRTP_FUNC(vm_block, bool, 5, int, int, int, int, int64_t);
    CRTP_FUNC(vm_call,  bool, 5, int, int, int, int, int64_t);

#undef CRTP_FUNC
};
#endif

