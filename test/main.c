#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <run.h>
#include <code.h>
#include <debug.h>


void print_uvalue(int type, union regvm_uvalue uv)
{
    switch (type)
    {
    case INTEGER:
        printf("%lld\n", (long long)uv.num);
        break;
    case STRING:
        printf("%s\n", uv.str);
        break;
    case DOUBLE:
        printf("%f\n", uv.dbl);
        break;
    default:
        printf("\n");
        break;
    }
}

void dump_reg_info(void* arg, const struct regvm_reg_info* info)
{
    switch ((intptr_t)info)
    {
    case 0:
        printf("\e[33m id\ttype\tvar\tvalue \e[0m\n");
        break;
    case -1:
        break;
    default:
        printf(" %d\t%d\t%p\t", info->id, info->type, info->from);
        print_uvalue(info->type, info->value);
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

int dump_trap_callback(struct regvm* vm, int type, int reg)
{
    switch (type)
    {
    case 0:
        regvm_debug_reg_callback(vm, dump_reg_info, NULL);
        break;
    case 1:
        regvm_debug_var_callback(vm, dump_var_info, NULL);
        break;
    default:
        return -1;
    }
    return 0;
}

#define RUN(...)                        \
    c = (struct code){__VA_ARGS__};     \
    regvm_exe_one(vm, &c);


int read_file(FILE* fp, struct regvm* vm)
{
    union 
    {
        char            data[10];
        code0_t         code0;
        code2_t         code2;
        code4_t         code4;
        code8_t         code8;
    }                   inst;

    char id[16];
    int type;
    int reg;

    char ex;
    char buf[1024];
    char data[1024];

    int read_bytes = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        if (buf[0] == '#') continue;

        buf[sizeof(buf) - 1] = '\0';
        data[0] = '\0';

        sscanf(buf, "%16s %c %d $%d %1024[^\n]", id, &ex, &type, &reg, data);
        id[sizeof(id) - 1] = '\0';
        data[sizeof(data) - 1] = '\0';

        memset(&inst, 0, sizeof(inst));

        int b = 2;
        int v = 0;
        switch (*(uint32_t*)id)
        {
#define FIX_LEN(k, v)                       \
        case k:                             \
            inst.code0.base.id = v;         \
            break;
#define EXT_STR(k, v)                       \
        case k:                             \
            inst.code8.base.id = v;         \
            b += 8;                         \
            inst.code8.str = data;          \
            break;
        case 0x00544553:
            switch (ex)
            {
            case '2':
                b += 2;
                inst.code2.base.id = SET2;
                sscanf(data, "%d", &v);
                inst.code2.num = v;
                break;
            case '4':
                b += 4;
                inst.code4.base.id = SET4;
                sscanf(data, "%d", &v);
                inst.code4.num = v;
                break;
            case '8':
                b += 8;
                inst.code8.base.id = SET8;
                sscanf(data, "%ld", &inst.code8.num);
                break;
            case 'S':
                b += 8;
                inst.code8.base.id = SET8;
                inst.code8.str = data;
                break;
            default:
                printf("\e[31m --- 0x%X : %s %c \e[0m\n", *(uint32_t*)id, id, ex);
                return -1;
            }
            break;
        case 0x50415254:
            inst.code8.base.id = TRAP;
            inst.code8.other = dump_trap_callback;
            break;
        case 0x524F5453:
            if (ex == 'S')
            {
                inst.code0.base.id = STORE8;
                inst.code8.str = data;
            }
            else
            {
                inst.code0.base.id = STORE;
            }
            break;
            //EXT_STR(0x524F5453, STORE)
            EXT_STR(0x44414F4C, LOAD);
            FIX_LEN(0x434F4C42, BLOCK);
            FIX_LEN(0x00504F4E, NOP);
#undef FIX_LEN
#undef EXT_STR
        default:
            printf("\e[31m --- 0x%X : %s \e[0m\n", *(uint32_t*)id, id);
            continue;
        }
        inst.code0.base.type = type;
        inst.code0.base.reg = reg;

        printf("--- ");
        for (unsigned int i = 0; i < sizeof(inst); i++)
        {
            printf(" %02X", (unsigned char)inst.data[i]);
        }
        code_base_t* c = &inst.code0.base;
        printf(" : %02d : %02d : %s : %d\n", inst.code0.base.id,  b, id, c->id);

        int r = regvm_exe_one(vm, &inst.code0);
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
    struct regvm* vm =  regvm_init();

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
