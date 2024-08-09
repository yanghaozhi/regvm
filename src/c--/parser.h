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

    const char* statement(const char* src, std::function<void (const token&)> cb = {});

    typedef std::function<selector::reg (parser*, int, const token&, const token&)> calc_t;
    const char* expression(const char* src, selector::reg& reg, int* end, calc_t calc);
    const char* expression(const char* src, selector::reg& reg);

    const char* call_func(const char* src, const token& name, std::vector<selector::reg>& rets);

    const char* comma(const char* src, std::vector<selector::reg>& rets);

    selector::reg token_2_reg(const token& tok);

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
};

