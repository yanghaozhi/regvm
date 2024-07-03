#pragma once

#include <stdint.h>

#include <map>
#include <list>

#include <debug.h>
#include <ext.h>

#include "scope.h"


namespace ext
{

class var;

struct regvm_mem : public regvm_crtp<regvm_mem>
{
public:
    typedef var     var_t;

    regvm_mem();

    var* add(const int type, const char* name);

    var* get(const char* name) const;

    void dump(regvm* vm, var_cb cb, void* arg, regvm_var_info* info) const;

#define CRTP_FUNC(name, ret, argc, ...)                                             \
    ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__));

    CRTP_FUNC(vm_init,  bool, 0);
    CRTP_FUNC(vm_exit,  bool, 0);
    CRTP_FUNC(vm_var,   core::var*, 1, int);
    CRTP_FUNC(vm_var,   core::var*, 2, int, const char*);

    CRTP_FUNC(vm_new,   bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_store, bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_load,  bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_block, bool, 3, const code_t, int, int64_t);
    CRTP_FUNC(vm_call,  bool, 3, const code_t, int, int64_t);

#undef CRTP_FUNC

private:
    struct context
    {
        context(int64_t frame);
        ~context();

        void enter_block();
        void leave_block();

        const int64_t       frame;
        std::list<scope>    scopes;
    };

    scope                   globals;
    std::list<context>      frames;
};

}

