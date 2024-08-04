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

struct regvm_mem : public regvm
{
public:
    typedef var     var_t;

    regvm_mem();

    bool store();

    var* add(const char* name, const int type, bool global);

    var* get(const char* name, bool global) const;

    void dump(regvm* vm, var_cb cb, void* arg, regvm_var_info* info) const;

#define CRTP_FUNC(name, ret, argc, ...)                                             \
    virtual ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__)) override;

    CRTP_FUNC(vm_init,  bool, 0);
    CRTP_FUNC(vm_exit,  bool, 0);
    CRTP_FUNC(vm_call,  bool, 3, code_t, int, int64_t);
    CRTP_FUNC(vm_var,   core::var*, 1, int);
    CRTP_FUNC(vm_var,   core::var*, 2, int, const char*);

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

    static int vm_CODE_LOAD(regvm* vm, code_t code, int offset, const void* extra);
    static int vm_CODE_STORE(regvm* vm, code_t code, int offset, const void* extra);
    static int vm_CODE_BLOCK(regvm* vm, code_t code, int offset, const void* extra);
};

}

