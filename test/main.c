#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <run.h>
#include <code.h>
#include <debug.h>

void dump_reg_info(void* arg, const struct regvm_reg_info* info)
{
    if (info->id == 1)
    {
        printf("\e[33m id\tvalue\ttype\tvar \e[0m\n");
    }
    printf(" %d\t%lld\t%d\t%p\n", info->id, (long long)info->value.num, info->type, info->from);
}

void dump_var_info(void* arg, const struct regvm_var_info* info)
{
    printf("%lld\t%d\t%d\t%d\t\e[32m%s @ %s @ %d\e[0m\n", (long long)info->value.num, info->type, info->reg, info->ref, info->name, info->func, info->scope);
}

int main(int argc, char** argv)
{
    struct regvm* vm =  regvm_init();

    struct code c = {.id = SET, .type = INTEGER, .reg = 1, .ext = 0, .value.num = 123};
    //memset(&c, 0, sizeof(c));
    //c.id = SET;
    //c.type = INTEGER;
    //c.reg = 1;
    //c.ext = 0;
    //c.value.num = 123;

    bool r = regvm_exe_one(vm, &c);
    printf("%d\n", r);

    r = regvm_debug_reg_callback(vm, dump_reg_info, NULL);


    c = (struct code){.id = STOREN, .type = 0, .reg = 1, .ext = 0, .value.str = "abc"};
    r = regvm_exe_one(vm, &c);
    printf("%d\n", r);

    r = regvm_debug_reg_callback(vm, dump_reg_info, NULL);

    c = (struct code){.id = LOAD, .type = 0, .reg = 2, .ext = 0, .value.str = "abc"};
    r = regvm_exe_one(vm, &c);
    printf("%d\n", r);

    r = regvm_debug_reg_callback(vm, dump_reg_info, NULL);

    r = regvm_debug_var_callback(vm, dump_var_info, NULL);

    regvm_exit(vm);

    return 0;
}
