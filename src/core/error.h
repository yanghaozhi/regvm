#pragma once


#include <stdio.h>
#include <stdarg.h>

#include <irq.h>
#include <debug.h>

#include "reg.h"
#include "frame.h"

#ifdef DEBUG
#define VM_ERROR(e, c, o, fmt, ...)                                             \
    vm->err.self.line = __LINE__;                                               \
    vm->err.self.file = __FILE__;                                               \
    vm->err.self.func = __func__;                                               \
    vm->err.set(vm, e, c, o, fmt, ##__VA_ARGS__);
#else
#define VM_ERROR(e, c, o, fmt, ...)    vm->err.set(vm, e, c, o, fmt, ##__VA_ARGS__);
#endif

namespace core
{

class error
{
public:
    error(void);

    int                 code;

    regvm_src_location  src;
    regvm_src_location  func;
#ifdef DEBUG
    regvm_src_location  self;
#endif

    char                reason[1024];

    void set(regvm* vm, int errcode, code_t code, int offset, const char* fmt, ...);

    //void print_stack();

    //template <typename T> static void scope_vars(const scope& scope, T cb)
    //{
    //    for (int i = 0; i < scope::size; i++)
    //    {
    //        if (scope.table[i].v != NULL)
    //        {
    //            cb(scope.id, scope.table[i].v);
    //        }

    //        auto it = scope.table[i].next;
    //        while (it != NULL)
    //        {
    //            for (int i = 0; i < scope::items::size; i++)
    //            {
    //                if (it->vars[i] == NULL)
    //                {
    //                    return;
    //                }
    //                if (it->vars[i] != NULL)
    //                {
    //                    cb(scope.id, it->vars[i]);
    //                }
    //            }
    //            it = it->next;
    //        }
    //    }
    //}
    template <typename T> static void reg_info(const reg& reg, T cb, regvm_reg_info* info)
    {
        //cb(reg.values[i].value.num, reg.values[i].from, reg.values[i].type);
        for (int i = 0; i < reg::SIZE; i++)
        {
            info->id = i;
            info->ref = (reg.values[i].from != NULL) ? reg.values[i].from->ref : -1;
            info->type = reg.values[i].type;
            info->value.sint = reg.values[i].value.sint;
            info->from = reg.values[i].from;
            info->attr = reg.values[i].need_free;
            cb(info);
        }
    }

    //template <typename T> static void ctx_vars(const context& ctx, T cb, regvm_var_info* info)
    //{
    //    const char* f = "global";
    //    auto func = [info, &f, cb](int id, var* v)
    //    {
    //        info->ref = v->ref;
    //        info->type = v->type;
    //        info->reg = v->reg;
    //        info->scope = id;
    //        info->value.sint = v->value.sint;
    //        info->name = v->name;
    //        info->func = f;
    //        info->var = v;
    //        cb(info);
    //    };

    //    for (const auto& it : ctx.scopes)
    //    {
    //        f = ctx.running->info.entry.func;
    //        scope_vars(it, func);
    //    }

    //    f = "global";
    //    scope_vars(ctx.globals, func);
    //}
};

}

