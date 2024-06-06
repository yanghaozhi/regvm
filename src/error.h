#pragma once

#include <debug.h>

#include "reg.h"
#include "context.h"

#ifdef DEBUG
#define ERROR(errcode, fmt, ...)                                                \
    SET_ERROR(errcode, fmt, ##__VA_ARGS__);                                     \
    vm->code.self.line = __LINE__;                                              \
    vm->code.self.file = __FILE__;                                              \
    vm->code.self.func = __func__;
#else
#define ERROR(errcode, fmt, ...)    SET_ERROR(errcode, fmt, ##__VA_ARGS__)
#endif


#define SET_ERROR(errcode, fmt, ...)                                            \
    vm->code.code = errcode;                                                    \
    snprintf(vm->code.reason, sizeof(vm->code.reason), fmt, ##__VA_ARGS__);     \
    vm->code.reason[sizeof(vm->code.reason) - 1] = '\0';                        \
    vm->code.func = *vm->ctx->func;                                             \
    if (vm->ctx->src != NULL)                                                   \
    {                                                                           \
        vm->code.src = *vm->ctx->src;                                           \
    }



class error
{
public:
    int             code;

    src_location    src;
    src_location    func;
#ifdef DEBUG
    src_location    self;
#endif

    char            reason[1024];

    //void print_stack();

    template <typename T> static void scope_vars(const scope& scope, T cb)
    {
        for (int i = 0; i < scope::size; i++)
        {
            if (scope.table[i].v != NULL)
            {
                cb(scope.id, scope.table[i].v);
            }

            auto it = scope.table[i].next;
            while (it != NULL)
            {
                for (int i = 0; i < scope::items::size; i++)
                {
                    if (it->vars[i] == NULL)
                    {
                        return;
                    }
                    if (it->vars[i] != NULL)
                    {
                        cb(scope.id, it->vars[i]);
                    }
                }
                it = it->next;
            }
        }
    }
    template <typename T> static void reg_info(const regs& reg, T cb, regvm_reg_info* info)
    {
        //cb(vm->reg.values[i].num, vm->reg.froms[i], vm->reg.types[i]);
        const int size = sizeof(reg.types);
        for (int i = 1; i < size; i++)
        {
            info->id = i;
            info->type = reg.types[i];
            info->value.num = reg.values[i].num;
            info->from = reg.froms[i];
            cb(info);
        }
    }

    template <typename T> static void ctx_vars(const context& ctx, T cb, regvm_var_info* info)
    {
        const char* f = "global";
        auto func = [info, &f, cb](int id, var* v)
        {
            info->ref = v->ref;
            info->type = v->type;
            info->reg = v->reg;
            info->scope = id;
            info->value.num = v->value.num;
            info->name = v->name;
            info->func = f;
            cb(info);
        };
        scope_vars(ctx.globals, func);

        int id = 1;
        for (const auto& it : ctx.scopes)
        {
            f = ctx.func.func;
            scope_vars(it, func);
            ++id;
        }
    }
};
