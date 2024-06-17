#include "frame.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "structs.h"

using namespace core;

frame::frame(frame* c, func* f) : running(f)
{
    //scopes.emplace_front(scopes.size() + 1);

    up = c;
    down = NULL;
    if (c != NULL)
    {
        c->down = this;
    }
}

frame::~frame()
{
}

