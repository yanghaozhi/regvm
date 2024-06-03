#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <run.h>
#include <code.h>
#include <debug.h>

int main(int argc, char** argv)
{
    struct regvm* vm =  regvm_init();

    struct code c;
    memset(&c, 0, sizeof(c));
    c.id = SET;
    c.type = INTEGER;
    c.reg = 1;
    c.ext = 0;
    c.value.num = 123;

    bool r = regvm_exe_one(vm, &c);
    printf("%d\n", r);

    uint64_t value;
    void* var;
    int type;
    r = regvm_debug_reg_info(vm, 1, &value, &var, &type);
    printf("%ld %d <= %p\n", value, type, var);

    regvm_exit(vm);

    return 0;
}
