#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <run.h>
#include <code.h>
#include <debug.h>

void dump_reg_info(void* arg, int id, int64_t value, void* var, int type)
{
    if (id == 1)
    {
        printf("\e[33m id\tvalue\ttype\tvar \e[0m\n");
    }
    printf(" %d\t%lld\t%d\t%p\n", id, (long long)value, type, var);
}

void dump_var_info(void* arg, int scope, const char* name, int64_t value, int type, int reg, int ref)
{
    printf("\e[32m%d\e[0m\t%s\t%lld\t%d\t%d\t%d\n", scope, name, (long long)value, type, reg, ref);
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
