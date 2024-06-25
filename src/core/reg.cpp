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
        if(values[i].need_free == true)
        {
            switch (values[i].type)
            {
            case TYPE_STRING:
                free((void*)values[i].value.str);
                break;
            default:
                break;
            }
        }
    }
}

bool reg::v::write(uint64_t v, int t, bool c)
{
    if (c == true)
    {
        clear();
    }

    need_free = false;
    type = t;
    value.uint = v;

    return true;
}

//template <typename T> bool core::regv<T>::clear()
//{
//    store();
//
//    if ((from == NULL) && (need_free == true))
//    {
//        switch (type)
//        {
//        case TYPE_STRING:
//            free((char*)value.str);
//            value.str = NULL;
//            break;
//        case TYPE_DICT:
//            break;
//        case TYPE_LIST:
//            break;
//        default:
//            break;
//        }
//    }
//
//    set_from(NULL);
//
//    return true;
//}

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
#ifdef NULL_AS_0
    case TYPE_NULL:
        return 0;
#endif
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
#ifdef NULL_AS_0
    case TYPE_NULL:
        return 0;
#endif
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
#ifdef NULL_AS_0
    case TYPE_NULL:
        return 0;
#endif
    default:
        assert(0);
        return -1;
    }
}

//template <typename T> bool core::regv<T>::store() const
//{
//    core::var<T>* v = from;
//    if (v == NULL)
//    {
//        return false;
//    }
//
//    //if ((v->type != type) || (v->reg != idx))
//    if (v->reg != this)
//    {
//        //ERROR(ERR_TYPE_MISMATCH, "store %d != %d", v->type, types[i]);
//        return false;
//    }
//
//    //v->value = value;
//    v->set_val(type, value);
//
//    return true;
//}
//
//template <typename T> bool core::regv<T>::set_from(core::var<T>* v)
//{
//    if (from != NULL)
//    {
//        from->set_reg(NULL);
//    }
//    if (v != NULL)
//    {
//        v->set_reg(this);
//    }
//    from = v;
//    return true;
//}

