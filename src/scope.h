#pragma once

#include <stdint.h>

struct var;

class scope
{
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
};
