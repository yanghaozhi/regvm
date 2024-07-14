#pragma once

#include <map>
#include <deque>
#include <vector>
#include <string_view>
#include <unordered_map>

#include "common.h"
#include "select.h"
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

    int                 lineno      = 0;
    std::deque<inst>   insts;

    bool add(op* func, ...);

    const char* next_token(const char* src, token& tok);

    const char* statement(const char* src, int* end = NULL);

    const char* expression(const char* src, select::reg& reg, int* end = NULL);

    const char* call_func(const char* src, const token& name, std::vector<select::reg>& rets);

    const char* comma(const char* src, std::vector<select::reg>& rets);

    int label_id    = 1;

    void set_label(int label, int reg);
    void finish_label(int label, uint32_t addr);

private:
    int depth      = 0;

    struct trie_tree
    {
        op*                         func    = NULL;
        std::map<int, trie_tree*>   next;
    };

    trie_tree*                                  parser_list;
    std::unordered_map<std::string_view, int>   keywords;   //  name : TOKEN_T

    std::multimap<int, inst*>                   pending_labels;
    std::map<int, uint32_t>                     labels;

    select::reg token_2_reg(const token& tok);
    int operator_level(int op) const;
    template <typename T, typename O> select::reg pop_and_calc(T& toks, O& ops);
};

