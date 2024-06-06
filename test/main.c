#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <run.h>
#include <code.h>
#include <debug.h>

void dump_reg_info(void* arg, const struct regvm_reg_info* info)
{
    if (info == NULL)
    {
        printf("\e[33m id\tvalue\ttype\tvar \e[0m\n");
    }
    else
    {
        printf(" %d\t%lld\t%d\t%p\n", info->id, (long long)info->value.num, info->type, info->from);
    }
}

void dump_var_info(void* arg, const struct regvm_var_info* info)
{
    if (info == NULL)
    {
        printf("\e[32m type\treg\tref\tname @ func @ scope\tvalue\e[0m\n");
    }
    else
    {
        printf(" %d\t%d\t%d\t%s @ %s @ %d\t", info->type, info->reg, info->ref, info->name, info->func, info->scope);
        switch (info->type)
        {
        case INTEGER:
            printf("%lld\n", (long long)info->value.num);
            break;
        case STRING:
            printf("%s\n", info->value.str);
            break;
        case DOUBLE:
            printf("%f\n", info->value.dbl);
            break;
        }
    }
}

#define RUN(...)                        \
    c = (struct code){__VA_ARGS__};     \
    regvm_exe_one(vm, &c);


int main(int argc, char** argv)
{
    struct regvm* vm =  regvm_init();
    struct code c = {0};
    //bool r = false;

    RUN(.id = SET, .type = INTEGER, .reg = 1, .ext = 0, .value.num = 123);

    regvm_debug_reg_callback(vm, dump_reg_info, NULL);

    RUN(.id = STORE, .type = 0, .reg = 1, .ext = 0, .value.str = "abc");

    regvm_debug_reg_callback(vm, dump_reg_info, NULL);

    RUN(.id = LOAD, .type = 0, .reg = 2, .ext = 0, .value.str = "abc");

    regvm_debug_reg_callback(vm, dump_reg_info, NULL);

    regvm_debug_var_callback(vm, dump_var_info, NULL);

    RUN(.id = BLOCK, .type = 1, .reg = 0, .ext = 0, .value.str = NULL);

    RUN(.id = SET, .type = STRING, .reg = 3, .ext = 0, .value.str = "def");

    RUN(.id = STORE, .type = 0, .reg = 3, .ext = 0, .value.str = "abc");

    regvm_debug_var_callback(vm, dump_var_info, NULL);

    RUN(.id = BLOCK, .type = 2, .reg = 0, .ext = 0, .value.str = NULL);

    regvm_debug_var_callback(vm, dump_var_info, NULL);

    regvm_exit(vm);

    return 0;
}
