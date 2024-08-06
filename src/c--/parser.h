#pragma once

#include <map>
#include <deque>
#include <vector>
#include <functional>
#include <string_view>
#include <unordered_map>

#include "selector.h"
#include "blocks.h"
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
        std::deque<inst>&  insts;
        parser*             p;

        op(parser* pp) : insts(pp->insts), p(pp)     {};
        virtual const char* go(const char* src, const token* toks, int count)    = 0;
    };

    int                     lineno      = 0;
    std::deque<inst>        insts;

    bool go(const char* src, std::deque<inst>& insts);

    bool add(op* func, ...);

    const char* next_token(const char* src, token& tok);

    const char* statement(const char* src, std::function<void (const token&)> cb = {});

    const char* expression(const char* src, selector::reg& reg, int* end = NULL);

    const char* call_func(const char* src, const token& name, std::vector<selector::reg>& rets);

    const char* comma(const char* src, std::vector<selector::reg>& rets);

private:
    int                             depth      = 0;
    const char*                     last_line   = NULL;

    struct trie_tree
    {
        op*                         func    = NULL;
        std::map<int, trie_tree*>   next;
    };

    trie_tree*                                  parser_list;
    std::unordered_map<std::string_view, int>   keywords;   //  name : TOKEN_T

    selector::reg token_2_reg(const token& tok);
    int operator_level(int op) const;
    template <typename T, typename O> selector::reg literally_optimize(T& toks, O& ops);
    template <typename T, typename O> selector::reg pop_and_calc(T& toks, O& ops);

    selector                        regs;
    blocks                          scopes;
};

