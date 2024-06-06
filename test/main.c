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
        printf("\e[32m type\treg\tref\tname @ func @ scope\tvalue\e[0m\n");
        break;
    case -1:
        break;
    default:
        printf(" %d\t%d\t%d\t%s @ %s @ %d\t", info->type, info->reg, info->ref, info->name, info->func, info->scope);
        print_uvalue(info->type, info->value);
        break;
    }
}

#define RUN(...)                        \
    c = (struct code){__VA_ARGS__};     \
    regvm_exe_one(vm, &c);


void read_file(FILE* fp, struct regvm* vm)
{
    union 
    {
        char            data[10];
        code2_t         code2;
        code4_t         code4;
        code6_t         code6;
        code10_t        code10;
    }                   inst;

    char id[16];
    int type;
    int reg;

    char ex;
    char buf[1024];
    char data[1024];

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        buf[sizeof(buf) - 1] = '\0';
        data[0] = '\0';

        sscanf(buf, "%16s %d %d %c %1024[^\n]", id, &type, &reg, &ex, data);
        id[sizeof(id) - 1] = '\0';
        data[sizeof(data) - 1] = '\0';
        if (id[0] == '#') continue;

        memset(&inst, 0, sizeof(inst));

        switch (*(uint32_t*)id)
        {
#define MAP(k, v)                       \
        case k:                         \
            inst.code2.base.id = v;     \
            break;
            //MAP(0x00544553, SET);
            MAP(0x524F5453, STORE);
            MAP(0x44414F4C, LOAD);
            MAP(0x434F4C42, BLOCK);
            MAP(0x00504F4E, NOP)
#undef MAP
        default:
            printf("\e[31m --- 0x%X : %s \e[0m\n", *(uint32_t*)id, id);
            continue;
        }
        inst.code2.base.type = type;
        inst.code2.base.reg = reg;

        int v = 0;
        int b = 2;
        switch (ex)
        {
        default:
        case 'O':
            break;
        case '2':
            sscanf(data, "%d", &v);
            inst.code4.num = v;
            b += 2;
            break;
        case '4':
            sscanf(data, "%d", &v);
            inst.code6.num = v;
            b += 4;
            break;
        case '8':
            sscanf(data, "%ld", &inst.code10.num);
            b += 8;
            break;
        case 'S':
            inst.code10.str = data;
            b += 8;
            break;
        }
        printf("--- ");
        for (unsigned int i = 0; i < sizeof(inst); i++)
        {
            printf(" %02X", (unsigned char)inst.data[i]);
        }
        code_base_t* c = &inst.code2.base;
        printf(" : %02d : %s : %d\n", b, id, c->id);

        int r = regvm_exe_one(vm, &inst.code2);
        if (r < 0)
        {
            return;
        }
        //printf("--- %d,%d,%d,%c,%s\n", inst.code.id, inst.code.type, inst.code.reg, ex, data);
    };
}


int main(int argc, char** argv)
{
    struct regvm* vm =  regvm_init();

    FILE* fp = stdin;
    if (argc > 1)
    {
        fp = fopen(argv[1], "r");
    }

    read_file(fp, vm);

    if (argc > 1)
    {
        fclose(fp);
    }

    //struct code c = {0};
    ////bool r = false;

    //RUN(.id = SET, .type = INTEGER, .reg = 1, .ext = 0, .value.num = 123);

    //regvm_debug_reg_callback(vm, dump_reg_info, NULL);

    //RUN(.id = STORE, .type = 0, .reg = 1, .ext = 0, .value.str = "abc");

    ////regvm_debug_reg_callback(vm, dump_reg_info, NULL);

    //RUN(.id = LOAD, .type = 0, .reg = 2, .ext = 0, .value.str = "abc");

    //regvm_debug_reg_callback(vm, dump_reg_info, NULL);

    //regvm_debug_var_callback(vm, dump_var_info, NULL);

    //RUN(.id = BLOCK, .type = 1, .reg = 0, .ext = 0, .value.str = NULL);

    //RUN(.id = SET, .type = STRING, .reg = 3, .ext = 0, .value.str = "def");

    //RUN(.id = STORE, .type = 0, .reg = 3, .ext = 0, .value.str = "abc");

    //regvm_debug_reg_callback(vm, dump_reg_info, NULL);
    //regvm_debug_var_callback(vm, dump_var_info, NULL);

    //RUN(.id = BLOCK, .type = 2, .reg = 0, .ext = 0, .value.str = NULL);

    //regvm_debug_var_callback(vm, dump_var_info, NULL);

    regvm_exit(vm);

    return 0;
}
