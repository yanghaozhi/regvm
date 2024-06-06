#pragma once

#include <stdint.h>
#include <stdlib.h>

struct var;

class scope
{
    friend class error;

public:
    scope(int id);
    scope(const scope&)     = delete;
    scope(const scope&&)    = delete;
    ~scope();

    const int               id;

    bool add(var* v);
    var* get(uint32_t key, const char* name, int len) const;

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
