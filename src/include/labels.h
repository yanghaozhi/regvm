#pragma once

#include <stdint.h>

#include <deque>

#include "inst.h"
#include "log.h"


template <typename T> class labels
{
public:
    labels()  {}

    void jump(const T& label, inst* code, int pos)
    {
        js.emplace_back(info{code, label, pos});
    }

    void label(const T& label, int pos)
    {
        if (pos < label_min)
        {
            label_min = pos;
        }
        if (pos > label_max)
        {
            label_max = pos;
        }
        ls.emplace(label, pos);
    }
    bool finish(insts_t& insts)
    {
        for (auto& it : js)
        {
            auto l = ls.find(it.label);
            if (l == ls.end())
            {
                LOGE("need to jump to label %d, but it is NOT exists !!!", it.label);
                return false;
            }

            if (l->second > it.pos)
            {
                static_cast<instv<CODE_JUMP>*>(it.code)->offset = calc_counts(insts, it.pos, l->second) + it.code->count();
            }
            else
            {
                static_cast<instv<CODE_JUMP>*>(it.code)->offset = -(calc_counts(insts, l->second, it.pos) - it.code->count());
            }
            LOGT("set dest of %s @ %d:(%d,%d) to %d : %d - %d", VIEW(it.code->name), it.pos, it.code->count(), js.front().code->count(), static_cast<instv<CODE_JUMP>*>(it.code)->offset, l->second, it.pos);
        }

        return true;
    }

private:
    struct info
    {
        inst*   code;
        T       label;
        int     pos;
        //int     to;
    };

    int64_t     label_min = 0xFFFFFFFFFF;
    int64_t     label_max = -1;

    std::vector<info>   js;
    std::map<T, int>    ls;

    int calc_counts(insts_t& insts, int begin, int end)
    {
        int r = 0;
        for (int i = begin; i < end; i++)
        {
            inst* p = insts[i];
            LOGT("%s - %d : %d", p->name, p->count(), p->id);
            r += p->count();
        }
        LOGT("--------------- %d -> %d = %d", begin, end, r);
        return r;
    }
};


