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

    CRTP_FUNC(vm_new,   bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_store, bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_load,  bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_block, bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_call,  bool, 3, const code_t, int, int64_t);

#undef CRTP_FUNC
};


#ifdef REGVM_EXT

#include "regvm_ext.h"

#else
struct regvm_core : public regvm_crtp<regvm_core>
{
#define CRTP_FUNC(name, ret, argc, ...)                                             \
    ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__));

    CRTP_FUNC(vm_init,  bool, 0);
    CRTP_FUNC(vm_exit,  bool, 0);
    CRTP_FUNC(vm_var,   core::var*, 1, int);

    CRTP_FUNC(vm_new,   bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_store, bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_load,  bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_block, bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_call,  bool, 3, const code_t, int, int64_t);

#undef CRTP_FUNC
};
#endif

