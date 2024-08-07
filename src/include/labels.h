#pragma once

#include <stdint.h>

#include <deque>

#include "inst.h"


template <typename T> class labels
{
public:
    labels()  {}

    void set_jump(const T& label, inst* code, int pos)
    {
        js.emplace_back(info{code, label, pos + 1, -1});
    }

    void set_label(const T& label, int pos)
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
    bool finish()
    {
        //int64_t diff = (std::max(label_max, (int64_t)js.back().pos) - std::min(label_min, (int64_t)js.front().pos)) * 2;

        //for (auto& it : js)
        //{
        //    auto l = ls.find(it.label);
        //    if (l == ls.end())
        //    {
        //        LOGE("need to jump to label %d, but it is NOT exists !!!", it.label);
        //        return false;
        //    }

        //    it.code->val.sint = diff;
        //    it.code->ex = TYPE_SIGNED;
        //    it.code->recalc();
        //    it.to = l->second;
        //}

        //for (auto& it : js)
        //{
        //    if (it.to > it.pos)
        //    {
        //        it.code->val.sint = (calc_bytes(it.pos, it.to) >> 1) + 1;
        //    }
        //    else
        //    {
        //        it.code->val.sint = -((calc_bytes(it.to, it.pos) - js.front().code->bytes) >> 1) - 1;
        //    }
        //}

        return true;
    }

private:
    struct info
    {
        inst*   code;
        T       label;
        int     pos;
        int     to;
    };

    int64_t     label_min = 0xFFFFFFFFFF;
    int64_t     label_max = -1;

    std::vector<info>   js;
    std::map<T, int>    ls;

    int calc_bytes(int begin, int end)
    {
        int r = 0;
        //auto b = insts.begin() + begin;
        //auto e = insts.begin() + end;
        //for (auto& p = b; p != e; ++p)
        //{
        //    LOGT("%s - %d : %d : %d", p->name, p->bytes >> 1, p->reg, p->ex);
        //    r += p->bytes;
        //}
        //LOGT("--------------- %d -> %d = %d", begin, end, r >> 1);
        return r;
    }
};


