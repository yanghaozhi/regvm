#include "scope.h"

#include <stdlib.h>
#include <string.h>

#include "var.h"


scope::scope()
{
    memset(table, 0, sizeof(table));
}

scope::~scope()
{
    for (int i = 0; i < size; i++)
    {
        clear_list(table[i].next);
    }
}

bool scope::add(var* v)
{
    const int i = v->attr.hash & 0x0F;
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

var* scope::get(uint32_t key, const char* name, int len)
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
