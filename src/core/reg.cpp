#include "reg.h"

#include <string.h>

#include "code.h"

#include "structs.h"
#include "error.h"

#ifdef REGVM_EXT
#include "regvm_ext.h"
#endif

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
            //values[i].from->release();
            values[i].set_from(NULL);
        }
        if(values[i].need_free == true)
        {
            switch (values[i].type)
            {
            case TYPE_STRING:
                free((void*)values[i].value.str);
                break;
            case TYPE_LIST:
                delete values[i].value.list_v;
                break;
            case TYPE_DICT:
                delete values[i].value.dict_v;
                break;
            default:
                break;
            }
        }
    }
}

bool reg::v::write(uint64_t v, int t, bool c)
{
    if ((c == true) || (need_free == true))
    {
        clear();
    }

    need_free = false;
    type = t;
    value.uint = v;

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

