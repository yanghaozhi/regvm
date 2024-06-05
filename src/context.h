#pragma once

#include <code.h>

struct var;

class context
{
public:
    var* add(const code* inst);

    var* get(const char* name);
};
