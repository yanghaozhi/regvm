#include <regvm.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "reg.h"
#include "func.h"

#define UNSUPPORT_TYPE(op, t, c, o) ERROR(ERR_TYPE_MISMATCH, c, o, "UNSUPPORT %s value type : %d", op, t); 

using namespace core;

extern "C"
{

bool regvm_exec(struct regvm* vm, const code_t* start, int count, int64_t* exit)
{
    if (vm->run(start, count) == false)
    {
        //TODO : ERROR
        //printf("\e[31m run ERROR at %d\e[0m\n", count - rest);
        return false;
    }
    *exit = vm->exit_code;
    return true;
}

int regvm_exec_step(struct regvm* vm, const code_t* code, int max)
{
    int next = 0;
    return (core::func::step(vm, code, 0, max, &next) == false) ? 0 : next;
}

int regvm_code_len(code_t code)
{
    int count = 1;
    switch (code.id)
    {
    case CODE_NOP:
        count += code.ex;
        break;
    case CODE_SETS:
        count += 1;
        break;
    case CODE_SETI:
        count += 2;
        break;
    case CODE_SETL:
        count += 4;
        break;
    case CODE_EXIT:
        break;
    default:
        if (code.id >= 0x80)
        {
            count += 1;
        }
        break;
    }
    return count;
}

}   //extern C

bool vm_set(struct regvm* vm, const code_t code, int offset, int64_t value)
{
    auto& r = vm->reg.id(code.reg);
    if ((code.ex == TYPE_STRING) && (value & 0x01))
    {
        //auto it = vm->strs.find(value);
        //if (it == vm->strs.end())
        //{
        //    ERROR(ERR_STRING_RELOCATE, code, offset, "need to relocate string : %ld", value);
        //    return false;
        //}
        //value = (intptr_t)it->second;
        auto& it = vm->idt.isrs[IRQ_STR_RELOCATE];
        if (it.func == NULL)
        {
            ERROR(ERR_STRING_RELOCATE, code, offset, "need to relocate string : %ld", value);
            return false;
        }
        value = it.call(vm, IRQ_STR_RELOCATE, code, offset, (void*)value);
        if (value == 0)
        {
            ERROR(ERR_STRING_RELOCATE, code, offset, "relocate string : %ld ERROR", value);
            return false;
        }
    }
    return r.write(value, code.ex, (code.ex != r.type));
}

bool vm_move(struct regvm* vm, const code_t code)
{
    auto& e = vm->reg.id(code.ex);
    auto& r = vm->reg.id(code.reg);
    return r.write(e.value.uint, e.type, true);
}

bool vm_clear(struct regvm* vm, const code_t code)
{
    auto& r = vm->reg.id(code.reg);
    return r.write(0, code.ex, true);
}

bool vm_conv_impl(struct regvm* vm, reg::v& r, int to)
{
    if (r.type == to) return true;

    switch (to)
    {
    case TYPE_UNSIGNED:
        r.value.uint = (uint64_t)r;
        break;
    case TYPE_SIGNED:
        r.value.sint = (int64_t)r;
        break;
    case TYPE_DOUBLE:
        r.value.dbl = (double)r;
        break;
    default:
        return false;
    }

    r.set_from(NULL);
    r.type = to;

    return true;
}

bool vm_conv(struct regvm* vm, const code_t code, int offset)
{
    auto& r = vm->reg.id(code.reg);
    if (vm_conv_impl(vm, r, code.ex) == false)
    {
        UNSUPPORT_TYPE("conv", code.ex, code, offset);
        return false;
    }
    return true;
}

bool vm_type(struct regvm* vm, const code_t code, int offset)
{
    auto& r = vm->reg.id(code.reg);
    auto& e = vm->reg.id(code.ex);
    return r.write(e.type, TYPE_UNSIGNED, true);
}

bool vm_chg(struct regvm* vm, const code_t code, int offset)
{
    auto& r = vm->reg.id(code.reg);
    switch (code.ex)
    {
    case 0: //clear
        r.value.uint = 0;
        return true;
    case 1: //minus
        switch (r.type)
        {
        case TYPE_UNSIGNED:
        case TYPE_SIGNED:
            r.value.sint = 0 - r.value.sint;
            break;
        case TYPE_DOUBLE:
            r.value.dbl = 0 - r.value.dbl;
            break;
        default:
            UNSUPPORT_TYPE("chg", r.type, code, offset);
            return false;
        }
        return true;
    case 2: //reciprocal
        switch (r.type)
        {
        case TYPE_UNSIGNED:
        case TYPE_SIGNED:
            if (vm_conv_impl(vm, r, TYPE_DOUBLE) == false)
            {
                UNSUPPORT_TYPE("chg", r.type, code, offset);
                return false;
            }
            [[fallthrough]];
        case TYPE_DOUBLE:
            r.value.dbl = 1 / r.value.dbl;
            break;
        default:
            UNSUPPORT_TYPE("chg", r.type, code, offset);
            return false;
        }
        return true;
    case 3: //NOT
        if (r.type == TYPE_UNSIGNED)
        {
            r.value.uint = ~r.value.uint;
            return true;
        }
        else
        {
            UNSUPPORT_TYPE("chg", r.type, code, offset);
            return false;
        }
    case 4: //malloc
        if (r.type == TYPE_STRING)
        {
            //r.value.uint = ~r.value.uint;
            if (r.need_free == false)
            {
                r.set_from(NULL);
                char* p = strdup(r.value.str);
                r.value.str = p;
                r.need_free = true;
            }
            return true;
        }
        else
        {
            UNSUPPORT_TYPE("chg", r.type, code, offset);
            return false;
        }
    default:
        UNSUPPORT_TYPE("chg", code.ex, code, offset);
        return false;
    }
}


int vm_jump(struct regvm* vm, const code_t code, int offset)
{
    auto& e = vm->reg.id(code.ex);
    switch (e.type)
    {
    case TYPE_SIGNED:
        return (int64_t)e;
    case TYPE_ADDR:
        return e.conv_i(TYPE_UNSIGNED) - offset;
    default:
        return 0;
    }
}

void vm_cmd_echo_var(const reg::v& v)
{
    switch (v.type)
    {
    case TYPE_SIGNED:
        printf("%lld", (long long)v.value.sint);
        break;
    case TYPE_UNSIGNED:
        printf("%llu", (unsigned long long)v.value.uint);
        break;
    case TYPE_STRING:
        printf("%s", v.value.str);
        break;
    case TYPE_DOUBLE:
        printf("%f", v.value.dbl);
        break;
    default:
        break;
    }
}

bool vm_cmd_echo(regvm* vm, int ret, const extend_args& args)
{
    switch (args.a1)
    {
    case 3:
        vm_cmd_echo_var(vm->reg.id(args.a2));
        printf("\t");
        vm_cmd_echo_var(vm->reg.id(args.a3));
        printf("\t");
        vm_cmd_echo_var(vm->reg.id(args.a4));
        break;
    case 2:
        vm_cmd_echo_var(vm->reg.id(args.a2));
        printf("\t");
        vm_cmd_echo_var(vm->reg.id(args.a3));
        break;
    case 1:
        vm_cmd_echo_var(vm->reg.id(args.a2));
        break;
    case 0:
        break;
    default:
        return false;
    }
    printf("\n");
    return true;
}

bool vm_cmd(regvm* vm, int ret, int op, const extend_args& args)
{
    switch (op)
    {
    case 0:
        return vm_cmd_echo(vm, ret, args);
    default:
        return false;
    }
}

bool vm_str_len(regvm* vm, int ret, reg::v& s, const extend_args& args)
{
    int len = strlen(s.value.str);
    return vm->reg.id(ret).write(len, TYPE_SIGNED, true);
}

bool vm_str_substr(regvm* vm, int ret, reg::v& r, const extend_args& args)
{
    auto& start = vm->reg.id(args.a2);
    auto& len = vm->reg.id(args.a3);

    const int l = (int64_t)len;
    char* p = (char*)malloc(l + 1);
    memcpy(p, r.value.str + (int64_t)start, l);
    p[l] = '\0';

    r.clear();

    r.need_free = true;
    r.value.str = p;
    r.type = TYPE_STRING;

    return true;
}

bool vm_str(regvm* vm, int ret, int op, reg::v& r, const extend_args& args)
{
    switch (op)
    {
    case 0:
        return vm_str_len(vm, ret, r, args);
    case 1:
        return vm_str_substr(vm, ret, r, args);
    default:
        return false;
    }
}

#undef UNSUPPORT_TYPE


