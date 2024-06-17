#include "reg.h"

#include <string.h>

#include "code.h"

#include "structs.h"
#include "error.h"

using namespace core;

reg::reg()
{
    memset(values, 0, sizeof(values));
    for (int i = 0; i < size; i++)
    {
        values[i].idx = i;
    }
}

reg::~reg()
{
    for (unsigned int i = 0; i < size; i++)
    {
        if (values[i].from != NULL)
        {
            values[i].from->release();
        }
    }
}

bool reg::v::set(uint64_t num, int ex)
{
    store();

    type = ex;
    value.uint = num;
    set_from(NULL);

    return true;
}

double reg::v::conv_d(int type) const
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
        assert(0);
        return -1;
    }
}

int64_t reg::v::conv_i(int type) const
{
    switch (type)
    {
    case TYPE_SIGNED:
    case TYPE_UNSIGNED:
    case TYPE_ADDR:
        return value.sint;
    case TYPE_DOUBLE:
        return (int64_t)value.dbl;
    default:
        assert(0);
        return -1;
    }
}

uint64_t reg::v::conv_u(int type) const
{
    switch (type)
    {
    case TYPE_SIGNED:
    case TYPE_UNSIGNED:
    case TYPE_ADDR:
        return value.uint;
    case TYPE_DOUBLE:
        return (uint64_t)value.dbl;
    default:
        assert(0);
        return -1;
    }
}

//reg::v& reg::v::neighbor(int id)
//{
//    v* o = this + (id - idx);
//    return *o;
//}

bool core::regv::store() const
{
    core::var* v = from;
    if (v == NULL)
    {
        return false;
    }

    //if ((v->type != type) || (v->reg != idx))
    if (v->reg != idx)
    {
        //ERROR(ERR_TYPE_MISMATCH, "store %d != %d", v->type, types[i]);
        return false;
    }

    //v->value = value;
    v->set_val(type, value);

    return true;
}

bool core::regv::set_from(core::var* v)
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

//double core::uvalue::conv(int type, double v) const
//{
//    switch (type)
//    {
//    case TYPE_SIGNED:
//        return (double)sint;
//    case TYPE_UNSIGNED:
//        return (double)uint;
//    case TYPE_DOUBLE:
//        return dbl;
//    default:
//        assert(0);
//        return -1;
//    }
//}
//
//int64_t core::uvalue::conv(int type, int64_t t) const
//{
//    switch (type)
//    {
//    case TYPE_SIGNED:
//    case TYPE_UNSIGNED:
//        return sint;
//    case TYPE_DOUBLE:
//        return (int64_t)dbl;
//    default:
//        assert(0);
//        return -1;
//    }
//}
//
//uint64_t core::uvalue::conv(int type, uint64_t u) const
//{
//    switch (type)
//    {
//    case TYPE_SIGNED:
//    case TYPE_UNSIGNED:
//        return uint;
//    case TYPE_DOUBLE:
//        return (uint64_t)dbl;
//    default:
//        assert(0);
//        return -1;
//    }
//}
//
