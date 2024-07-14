#pragma once

#include <map>
#include <deque>
#include <vector>
#include <string_view>
#include <unordered_map>

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

    int                 lineno      = 0;
    std::deque<inst>   insts;

    bool add(op* func, ...);

    const char* next_token(const char* src, token& tok);

    const char* statement(const char* src, int* end = NULL);

    const char* expression(const char* src, int& reg, int* end = NULL);

    const char* call_func(const char* src, const token& name, int& count, int8_t* rets);

    const char* comma(const char* src, std::vector<int>& rets);

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

    int token_2_reg(const token& tok);
    int operator_level(int op) const;
    template <typename T, typename O> int pop_and_calc(T& toks, O& ops);
};

