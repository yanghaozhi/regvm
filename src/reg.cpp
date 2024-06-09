#include "reg.h"

#include <string.h>

#include "code.h"

#include "var.h"
#include "error.h"

regs::regs()
{
    memset(values, 0, sizeof(values));
    for (int i = 0; i < size; i++)
    {
        values[i].idx = i;
    }
}

regs::~regs()
{
    for (unsigned int i = 0; i < size; i++)
    {
        if (values[i].from != NULL)
        {
            values[i].from->release();
        }
    }
}

bool regs::reg_v::set(uint64_t num, int ex)
{
    store();

    type = ex;
    value.uint = num;
    set_from(NULL);

    return true;
}

bool regs::reg_v::load(var* v)
{
    if (v == NULL) return false;

    if (v->reg == idx)
    {
        type = v->type;
        value = v->value;
        return true;
    }

    store();

    if (v->reg >= 0)
    {
        auto& o = neighbor(v->reg);
        o.store();
        o.set_from(NULL);
        //values[o].from->set_reg(-1);
        //values[o].from = NULL;
    }

    type = v->type;
    value = v->value;
    set_from(v);

    return true;
}

bool regs::reg_v::store() const
{
    var* v = from;
    if (v == NULL)
    {
        return false;
    }

    if ((v->type != type) || (v->reg != idx))
    {
        //ERROR(ERR_TYPE_MISMATCH, "store %d != %d", v->type, types[i]);
        return false;
    }

    v->value = value;

    return true;
}

bool regs::reg_v::store(var* v)
{
    if (v->reg == idx) return store();

    if (v == NULL) return false;

    if (v->type != type)
    {
        //TODO : error handler
        return false;
    }

    var* old = from;
    if (old != NULL)
    {
        //do NOT writeback
        old->release();
    }

    //v->acquire();

    set_from(v);

    v->value = value;
    v->reg = idx;

    return true;
}

regs::reg_v::operator double () const
{
    switch (type)
    {
    case TYPE_SIGNED:
        return (double)value.sint;
    case TYPE_UNSIGNED:
        return (double)value.uint;
    case TYPE_DOUBLE:
        return value.dbl;
    default:
        //TODO
        return -1;
    }
}

regs::reg_v::operator int64_t () const
{
    switch (type)
    {
    case TYPE_SIGNED:
    case TYPE_UNSIGNED:
        return value.sint;
    case TYPE_DOUBLE:
        return (int64_t)value.dbl;
    default:
        //TODO
        return -1;
    }
}

regs::reg_v::operator uint64_t () const
{
    switch (type)
    {
    case TYPE_SIGNED:
    case TYPE_UNSIGNED:
        return value.uint;
    case TYPE_DOUBLE:
        return (uint64_t)value.dbl;
    default:
        //TODO
        return -1;
    }
}

regs::reg_v& regs::reg_v::neighbor(int id)
{
    reg_v* o = this + (id - idx);
    return *o;
}

bool regs::reg_v::set_from(var* v)
{
    if (from != NULL)
    {
        from->set_reg(-1);
    }
    if (v != NULL)
    {
        v->set_reg(idx);
    }
    from = v;
    return true;
}
