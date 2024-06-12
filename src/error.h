#pragma once

#include <stdio.h>

#include <irq.h>
#include <debug.h>

#include "reg.h"
#include "context.h"

#ifdef DEBUG
#define ERROR(e, c, o, fmt, ...)                                                \
    SET_ERROR(e, c, o, fmt, ##__VA_ARGS__);                                     \
    vm->err.self.line = __LINE__;                                               \
    vm->err.self.file = __FILE__;                                               \
    vm->err.self.func = __func__;
#else
#define ERROR(errcode, fmt, ...)    SET_ERROR(errcode, fmt, ##__VA_ARGS__)
#endif

#define SET_ERROR(e, c, o, fmt, ...)                                            \
    vm->err.code = e;                                                           \
    snprintf(vm->err.reason, sizeof(vm->err.reason), fmt, ##__VA_ARGS__);       \
    vm->err.reason[sizeof(vm->err.reason) - 1] = '\0';                          \
    vm->err.func = vm->ctx->running->info.entry;                                \
    {                                                                           \
        regvm_error err;                                                        \
        if (vm->ctx->cur != NULL)                                               \
        {                                                                       \
            vm->err.src = *vm->ctx->cur;                                        \
            err.src = &vm->err.src;                                             \
        }                                                                       \
        err.code = e;                                                           \
        err.reason = vm->err.reason;                                            \
        vm->idt.call(vm, IRQ_ERROR, c, o, &err);                                \
    }


class error
{
public:
    bool                fatal   = false;
    int64_t             exit    = 0;
    int                 code;

    regvm_src_location  src;
    regvm_src_location  func;
#ifdef DEBUG
    regvm_src_location  self;
#endif

    char                reason[1024];

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
        //cb(vm->reg.values[i].value.num, vm->reg.values[i].from, vm->reg.values[i].type);
        for (int i = 0; i < regs::size; i++)
        {
            info->id = i;
            info->ref = (reg.values[i].from != NULL) ? reg.values[i].from->ref : -1;
            info->type = reg.values[i].type;
            info->value.sint = reg.values[i].value.sint;
            info->from = reg.values[i].from;
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
            info->value.sint = v->value.sint;
            info->name = v->name;
            info->func = f;
            info->var = v;
            cb(info);
        };

        for (const auto& it : ctx.scopes)
        {
            f = ctx.running->info.entry.func;
            scope_vars(it, func);
        }

        f = "global";
        scope_vars(ctx.globals, func);
    }
};
