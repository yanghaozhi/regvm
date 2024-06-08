#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <irq.h>
#include <code.h>
#include <regvm.h>
#include <debug.h>


void print_uvalue(int type, union regvm_uvalue uv)
{
    switch (type)
    {
    case TYPE_SIGNED:
        printf("%lld\n", (long long)uv.sint);
        break;
    case TYPE_UNSIGNED:
        printf("%llu\n", (unsigned long long)uv.uint);
        break;
    case TYPE_STRING:
        printf("%s\n", uv.str);
        break;
    case TYPE_DOUBLE:
        printf("%f\n", uv.dbl);
        break;
    default:
        printf("\n");
        break;
    }
}

struct dump_arg
{
    int     reg;
};

void dump_reg_info(void* arg, const struct regvm_reg_info* info)
{
    struct dump_arg* p = arg;

    switch ((intptr_t)info)
    {
    case 0:
        printf("\e[33m id\ttype\tref\tvar\tvalue \e[0m\n");
        break;
    case -1:
        break;
    default:
        if ((p->reg < 0) || (p->reg == info->id))
        {
            printf(" %d\t%d\t%d\t%p\t", info->id, info->type, info->ref, info->from);
            print_uvalue(info->type, info->value);
        }
        break;
    }
}

void dump_var_info(void* arg, const struct regvm_var_info* info)
{
    switch ((intptr_t)info)
    {
    case 0:
        printf("\e[32m type\treg\tref\tname @ func @ scope\tptr\tvalue\e[0m\n");
        break;
    case -1:
        break;
    default:
        printf(" %d\t%d\t%d\t%s @ %s @ %d\t%p\t", info->type, info->reg, info->ref, info->name, info->func, info->scope, info->var);
        print_uvalue(info->type, info->value);
        break;
    }
}

int dump_trap_callback(struct regvm* vm, int irq, int reg, int ex)
{
    struct dump_arg arg = {0};
    switch (ex)
    {
    case 0: //all regs
        arg.reg = -1;
        regvm_debug_reg_callback(vm, dump_reg_info, &arg);
        break;
    case 1: //1 arg
        arg.reg = reg;
        regvm_debug_reg_callback(vm, dump_reg_info, &arg);
        break;
    case 2:
        regvm_debug_var_callback(vm, dump_var_info, &arg);
        break;
    default:
        return -1;
    }
    return 0;
}

int dump_error_callback(struct regvm* vm, int irq, int code, const char* reason)
{
    printf("\e[31m %d - %s \e[0m\n", code, reason);
    return 0;
}


int read_file(FILE* fp, struct regvm* vm)
{
    union 
    {
        char        data[10];
        code_t      code;
    }               inst;

    union
    {
        char        s[8];
        uint64_t    v;
    }               id;
    int             ex;
    int             reg;

    char buf[1024];
    char data[1024];

    int read_bytes = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        id.v = 0;
        if (buf[0] == '#') continue;

        buf[sizeof(buf) - 1] = '\0';
        //data[0] = '\0';

        sscanf(buf, "%7s %d %d %1024[^\n]", id.s, &reg, &ex, data);
        data[sizeof(data) - 1] = '\0';

        memset(&inst, 0, sizeof(inst));

        int b = 2;
        switch (id.v)
        {
#define SINGLE(k, v)                        \
        case k:                             \
            inst.code.id = CODE_##v;        \
            break;
        SINGLE(0x50415254, TRAP);
        SINGLE(0x45524F5453, STORE);
        SINGLE(0x44414F4C, LOAD);
        SINGLE(0x4B434F4C42, BLOCK);
        SINGLE(0x434E49, INC);
        SINGLE(0x434544, DEC);
        SINGLE(0x444441, ADD);
        SINGLE(0x425553, SUB);
#undef SINGLE
        case 0x53544553:
            inst.code.id = CODE_SETS;
            {
                int v = 0;
                sscanf(data, "%d", &v);
                *(int16_t*)(&inst.code + 1) = v;
                b += 2;
            }
            break;
        case 0x49544553:
            inst.code.id = CODE_SETI;
            sscanf(data, "%d", (int32_t*)(&inst.code + 1));
            b += 4;
            break;
        case 0x4C544553:
            inst.code.id = CODE_SETL;
            sscanf(data, "%ld", (int64_t*)(&inst.code + 1));
            b += 8;
            break;
        case 0x43544553:
            inst.code.id = CODE_SETL;
            *(intptr_t*)(&inst.code + 1) = (intptr_t)data;
            break;
        default:
            printf("\e[31m --- 0x%lX : %s \e[0m\n", id.v, id.s);
            continue;
        }
        inst.code.ex = ex;
        inst.code.reg = reg;

        printf("--- ");
        for (unsigned int i = 0; i < sizeof(inst); i++)
        {
            printf(" %02X", (unsigned char)inst.data[i]);
        }
        printf(" : %02d : %02d : %s\n", inst.code.id, b, id.s);

        int r = regvm_exe_one(vm, &inst.code, sizeof(inst));
        if (r < 0)
        {
            return -1;
        }
        read_bytes += b;
    };
    return read_bytes;
}


int main(int argc, char** argv)
{
    if (argc == 1)
    {
        printf("Need input code file\n");
        return 0;
    }

    struct regvm* vm =  regvm_init();

    regvm_irq_set(vm, IRQ_TRAP, dump_trap_callback);

    FILE* fp = stdin;
    if (argc > 1)
    {
        fp = fopen(argv[1], "r");
    }

    int r = read_file(fp, vm);
    printf("\n\n%d bytes total\n", r);

    if (argc > 1)
    {
        fclose(fp);
    }

    regvm_exit(vm);

    return 0;
}
