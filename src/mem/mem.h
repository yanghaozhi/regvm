#pragma once

#include <stdint.h>

#include <map>
#include <vector>
#include <unordered_map>

#include <debug.h>
#include <ext.h>


namespace ext
{

class var;

struct regvm_mem : public regvm
{
public:
    typedef var     var_t;

    regvm_mem();

    bool store();

    var* add(uint64_t id, const int type);

    var* get(uint64_t id) const;

    bool del(uint64_t id);
    bool del(uint64_t first, uint64_t last);

    void dump(regvm* vm, var_cb cb, void* arg, regvm_var_info* info) const;

#define CRTP_FUNC(name, ret, argc, ...)                                             \
    virtual ret name(MLIB_MULTI_0_EXT(MLIB_DECL_GEN, argc, __VA_ARGS__)) override;

    CRTP_FUNC(vm_init,  bool, 0);
    CRTP_FUNC(vm_exit,  bool, 0);
    CRTP_FUNC(vm_call,  bool, 3, code_t, int, int64_t);
    CRTP_FUNC(vm_var,   core::var*, 1, int);
    CRTP_FUNC(vm_var,   core::var*, 2, int, uint64_t);

#undef CRTP_FUNC

    static int vm_CODE_LOAD(regvm* vm, code_t code, int offset, const void* extra);
    static int vm_CODE_STORE(regvm* vm, code_t code, int offset, const void* extra);
    static int vm_CODE_BLOCK(regvm* vm, code_t code, int offset, const void* extra);

private:
    uint64_t                    cur_call;
    std::map<uint64_t, var_t*>  vars;
    std::vector<int64_t>        calls;

    inline uint64_t var_id(uint64_t id) const
    {
        return cur_call + id;
    }

};

}

