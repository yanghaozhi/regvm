#pragma once

#include <stdint.h>

struct var;

class scope
{
public:
    scope();
    ~scope();

    bool add(var* v);
    var* get(uint64_t key, const char* name, int len);
    //bool del(uint64_t key, const char* name, int len);

private:
    struct items
    {
        struct items*       next;
        static const int    size    = 7;
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
