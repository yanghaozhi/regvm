#pragma once

#include <map>
#include <deque>
#include <vector>
#include <functional>
#include <string_view>
#include <unordered_map>

#include "selector.h"
#include "blocks.h"
#include "labels.h"
#include "common.h"
#include "inst.h"


#define INST(c, ...)   insts->emplace_back(new instv<CODE_##c>(#c, ##__VA_ARGS__));

class parser;

class func
{
public:
    func(parser* p);
    ~func();

    const int32_t           id;
    parser*                 parse       = NULL;
    //insts_t                 codes;
    insts_t*                insts;
    selector&               regs;
    blocks                  scopes;

    struct variable
    {
        std::string_view    name;
        DATA_TYPE           type        = TYPE_NULL;
        int                 attr        = 0;
    };

    int                     arg_begin   = -1;
    int                     ret_begin   = -1;

    std::vector<variable>   args;
    std::vector<variable>   rets;       //for multi return value extension

    const char* go(const char* src);

    bool fill_var(const token& tok, variable& var);

    template <typename T> const char* statements(const char* src, labels<T>& jump, const T& break_label, const T& continue_label)
    {
        auto cb = [this, &jump, &break_label, &continue_label](auto& tok)
        {
            switch (tok.info.type)
            {
            case Break:
                INST(JUMP, 0);
                jump.jump(break_label, insts->back(), insts->size());
                break;
            case Continue:
                INST(JUMP, 0);
                jump.jump(continue_label, insts->back(), insts->size());
                break;
            default:
                break;
            }
        };
        return statements(src, cb);
    }

    const char* statements(const char* src, std::function<void (const token&)> cb);
    const char* statement(const char* src);

    typedef std::function<selector::reg (func*, int, const token&, const token*, const selector::reg*)> calc_t;
    const char* expression(const char* src, selector::reg& reg, int* end, const calc_t& calc);
    const char* expression(const char* src, selector::reg& reg);

    const char* call_func(const char* src, const token& name, std::vector<selector::reg>& rets);

    template <typename F> const char* comma(const char* src, F&& func)
    {
        while ((src != NULL) && (*src != '\0'))
        {
            int end = -1;
            src = func(src, &end);

            switch (end)
            {
            case ',':
                break;
            case ';':
                LOGW("will comma expression end with ; ?");
                [[fallthrough]];
            case ')':
                return src;
            default:
                LOGE("comma expression should NOT end with %d - %c", end, (char)end);
                return NULL;
            }
        }
        return src;
    }

    selector::reg token_2_reg(const token& tok);

private:
    const char*                     last_line   = NULL;

    const char* statement(const char* src, std::function<void (const token&)> cb, token& tok);
};

