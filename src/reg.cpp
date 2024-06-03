#include "reg.h"

#include "code.h"

bool regs::set(const int id, const code* inst)
{
    if (valid_id(id) == false) return false;

    clear(id);

    types[id] = inst->type;
    froms[id] = NULL;
    values[id].num = inst->value.num;

    return true;
}

bool regs::set(const int id, var* v)
{
    if (valid_id(id) == false) return false;

    clear(id);

    if (v->acquire(id) == false)
    {
        return false;
    }

    types[id] = v->type;
    froms[id] = v;
    values[id] = v->value;

    return true;
}

void regs::clear(const int id)
{
    var* v = froms[id];
    if (v == NULL)
    {
        return;
    }
    if ((v->type != types[id]) || (v->reg != id))
    {
        //TODO : error handler
        return;
    }

    v->value = values[id];

    v->release();
}
