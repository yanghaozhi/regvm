#include "frame.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "structs.h"

using namespace core;

frame::frame(frame* c, func* f)
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

//void frame::enter_block()
//{
//    //scopes.emplace_front(scopes.size() + 1);
//}
//
//void frame::leave_block()
//{
//    //scopes.pop_front();
//}

//var* frame::add(const int type, const char* name)
//{
//    auto v = var::create(type, name);
//    scopes.front().add(v);
//    return (v->release() == true) ? v : NULL;
//}
//
//var* frame::get(const char* name) const
//{
//    const int l = strlen(name);
//    uint32_t h = var::calc_hash(name, l);
//    for (const auto& it : scopes)
//    {
//        auto v = it.get(h, name, l);
//        if (v != NULL)
//        {
//            return v;
//        }
//    }
//    return globals.get(h, name, l);
//}

