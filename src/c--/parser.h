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

class parser
{
public:
    parser();
    ~parser();

    class op
    {
    public:
        insts_t&            insts;
        parser*             p;

        op(parser* pp) : insts(pp->insts), p(pp)     {};
        virtual const char* go(const char* src, const token* toks, int count)    = 0;
    };

    int                     lineno      = 0;
    insts_t                 insts;
    selector                regs;
    blocks                  scopes;

    bool go(const char* src, insts_t& insts);

    bool add(op* func, ...);

    const char* next_token(const char* src, token& tok);

    template <typename T> const char* statements(const char* src, labels<T>& jump, const T& break_label, const T& continue_label)
    {
        auto cb = [this, &jump, &break_label, &continue_label](auto& tok)
        {
            switch (tok.info.type)
            {
            case Break:
                INST(JUMP, 0);
                jump.jump(break_label, insts.back(), insts.size());
                break;
            case Continue:
                INST(JUMP, 0);
                jump.jump(continue_label, insts.back(), insts.size());
                break;
            default:
                break;
            }
        };
        return statement(src, cb);
    }

    const char* statement(const char* src, std::function<void (const token&)> cb = {});

    typedef std::function<selector::reg (parser*, int, const token&, const token&)> calc_t;
    const char* expression(const char* src, selector::reg& reg, int* end, const calc_t& calc);
    const char* expression(const char* src, selector::reg& reg);

    const char* call_func(const char* src, const token& name, std::vector<selector::reg>& rets);

    const char* comma(const char* src, std::vector<selector::reg>& rets);

    selector::reg token_2_reg(const token& tok);

private:
    int                             depth       = 0;
    const char*                     last_line   = NULL;

    struct trie_tree
    {
        op*                         func        = NULL;
        std::map<int, trie_tree*>   next;
    };

    trie_tree*                                  parser_list;
    std::unordered_map<std::string_view, int>   keywords;   //  name : TOKEN_T
    
    const char* find_statement(const char* src, trie_tree* cur, token* toks, int idx, int max);
};

