#include "reg.h"

#include "code.h"

#include "var.h"

bool regs::set(const int id, const code* inst)
{
    if (valid_id(id) == false) return false;

    store(id);

    types[id] = inst->type;
    froms[id] = NULL;
    values[id].num = inst->value.num;

    return true;
}

bool regs::load(const int id, var* v)
{
    if ((v == NULL) || (valid_id(id) == false)) return false;

    store(id);

    v->set_reg(id);

    types[id] = v->type;
    froms[id] = v;
    values[id] = v->value;

    return true;
}

bool regs::store(const int id)
{
    if (valid_id(id) == false) return false;

    var* v = froms[id];
    if (v == NULL)
    {
        return true;
    }
    if ((v->type != types[id]) || (v->reg != id))
    {
        //TODO : error handler
        return false;
    }

    v->value = values[id];

    //froms[id] = NULL;
    //v->release();

    return true;
}

bool regs::store(const int id, var* v)
{
    if ((v == NULL) || (valid_id(id) == false)) return false;

    if (v->reg == id) return store(id);

    if (v->type != types[id])
    {
        //TODO : error handler
        return false;
    }

    var* old = froms[id];
    if (old != NULL)
    {
        //do NOT writeback
        old->release();
    }

    froms[id] = v;
    v->set_reg(id);

    v->value = values[id];
    v->reg = id;

    return true;
}

