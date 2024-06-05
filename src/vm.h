#include <run.h>

#include <stdlib.h>
#include <string.h>

#include "reg.h"
#include "context.h"

struct regvm
{
    regs        reg;
    context*    ctx;
};


