#include "reg.h"

#include <string.h>

#include "code.h"

#include "var.h"

regs::regs()
{
    memset(values, 0, sizeof(values));
    memset(froms, 0, sizeof(froms));
    memset(types, 0, sizeof(types));
}

regs::~regs()
{
    for (unsigned int i = 1; i < sizeof(types); i++)
    {
        if (froms[i] != NULL)
        {
            froms[i]->release();
        }
    }
}

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

    const auto r = v->reg;
    if (r != 0)
    {
        store(r);
        froms[r] = NULL;
    }

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

uint8_t regs::type(const int id)
{
    if (valid_id(id) == false) return false;
    return types[id];
}
