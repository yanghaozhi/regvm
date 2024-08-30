#pragma once

#include <stdint.h>

#include <map>
#include <vector>
#include <unordered_map>

#include <debug.h>
#include <ext.h>

#include <vm.h>

extern void mem_init(void);

namespace ext
{

class var;

struct regvm_mem : public regvm::ext
{
public:
    typedef var     var_t;

    regvm_mem();
    virtual ~regvm_mem();

    bool store();

    var* add(uint64_t id, const int type);

    var* get(uint64_t id) const;

    bool del(uint64_t id);
    bool del(uint64_t first, uint64_t last);

    void dump(regvm* vm, var_cb cb, void* arg, regvm_var_info* info) const;

    static bool init(regvm* vm, int idx, void* arg);
    static bool exit(regvm* vm, int idx, void* arg);

    static core::var* var_create_from_reg(regvm* vm, int id);
    static core::var* var_create(regvm* vm, int type, uint64_t id);

    bool vm_call(code_t code, int offset, int64_t id);

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

    template <typename F> inline void scan_local_vars(uint64_t id, F&& func);
};

}

