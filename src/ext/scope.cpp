#include "scope.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "var.h"

using namespace ext;

scope::scope(int32_t i) : id(i)
{
    memset(table, 0, sizeof(table));
}

scope::~scope()
{
    for (int i = 0; i < size; i++)
    {
        if (table[i].v != NULL)
        {
            if (table[i].v->reg != NULL)
            {
                table[i].v->reg->set_from(NULL);
            }
            table[i].v->release();
        }
        clear_list(table[i].next);
    }
}

bool scope::add(var* v)
{
    const int i = v->hash & 0x0F;
    if (table[i].v == NULL)
    {
        table[i].v = v;
        v->acquire();
        return true;
    }

    if (table[i].next == NULL)
    {
        table[i].next = (items*)malloc(sizeof(items));
        memset(table[i].next, 0, sizeof(items));
    }

    push_list(table[i].next, v);
    v->acquire();
    return true;
}

var* scope::get(uint32_t key, const char* name, int len) const
{
    const int i = key & 0x0F;
    if (table[i].v == NULL)
    {
        return NULL;
    }

    if (table[i].v->cmp(key, name, len) == true)
    {
        return table[i].v;
    }

    auto it = table[i].next;
    while (it != NULL)
    {
        for (int i = 0; i < items::size; i++)
        {
            if (it->vars[i] == NULL)
            {
                return NULL;
            }
            if (it->vars[i]->cmp(key, name, len) == true)
            {
                return it->vars[i];
            }
        }
        it = it->next;
    }

    return NULL;
}

void scope::push_list(items* it, var* v)
{
    if (it->next == NULL)
    {
        if (it->vars[items::size - 1] == NULL)
        {
            for (int i = 0; i < items::size; i++)
            {
                if (it->vars[i] == NULL)
                {
                    it->vars[i] = v;
                    return;
                }
            }
        }
        else
        {
            it->next = (items*)malloc(sizeof(items));
            memset(it->next, 0, sizeof(items));
        }
    }
    push_list(it->next, v);
}

void scope::clear_list(items* it)
{
    if (it == NULL) return;

    if (it->next != NULL)
    {
        clear_list(it->next);
        free(it->next);
    }

    for (int i = 0; i < items::size; i++)
    {
        if (it->vars[i] != NULL)
        {
            it->vars[i]->release();
        }
    }
}

void scope::dump(var_cb cb, void* arg, regvm_var_info* info) const
{
    auto call = [info, arg, cb](int id, var* v)
    {
        info->ref = v->ref;
        info->type = v->type;
        info->reg = (v->reg != NULL) ? v->reg->idx : -1;
        info->scope_id = id;
        info->value.sint = v->value.sint;
        info->var_name = v->name;
        info->raw = v;
        cb(arg, info);
    };

    for (int i = 0; i < size; i++)
    {
        if (table[i].v != NULL)
        {
            call(id, table[i].v);
        }

        auto it = table[i].next;
        while (it != NULL)
        {
            for (int i = 0; i < items::size; i++)
            {
                if (it->vars[i] == NULL)
                {
                    return;
                }
                if (it->vars[i] != NULL)
                {
                    call(id, it->vars[i]);
                }
            }
            it = it->next;
        }
    }
}
