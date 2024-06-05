#pragma once

#include <stdint.h>
#include <stdlib.h>

struct var;

class scope
{
    friend class debug;

public:
    scope();
    scope(const scope&)     = delete;
    scope(const scope&&)    = delete;
    ~scope();

    bool add(var* v);
    var* get(uint32_t key, const char* name, int len);

private:
    struct items
    {
        static const int    size    = 7;

        struct items*       next;
        struct var*         vars[size];
    };

    static const int        size    = 16;

    struct 
    {
        struct var*         v;
        struct items*       next;
    }                       table[size];

    void clear_list(items* it);
    void push_list(items* it, var* v);

    template <typename T> void for_each(T cb)
    {
        for (int i = 0; i < size; i++)
        {
            cb(table[i].v);

            auto it = table[i].next;
            while (it != NULL)
            {
                for (int i = 0; i < items::size; i++)
                {
                    if (it->vars[i] == NULL)
                    {
                        return;
                    }
                    cb(it->vars[i]);
                }
                it = it->next;
            }
        }
    }
};
