#include <run.h>

#include <stdlib.h>
#include <string.h>

#include "reg.h"
#include "error.h"
#include "scope.h"
#include "context.h"

struct regvm
{
    regs        reg;
    scope       globals;
    context*    ctx;

    regvm();
    ~regvm();

    bool call(void);
    bool ret(void);
};


