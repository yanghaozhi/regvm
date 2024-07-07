#pragma once

#include <map>
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
        virtual const char* go(parser* p, const char* src, const token* toks, int count)    = 0;
    };

    int                 lineno      = 0;
    std::vector<inst>   insts;

    const char* next_token(const char* src, token& tok);

    const char* statement(const char* src);

    bool add(op* func, ...);

    const char* expression(const char* src, int pri, bool& fin, int& reg);

private:
    int depth      = 0;

    struct trie_tree
    {
        op*                         func    = NULL;
        std::map<int, trie_tree*>   next;
    };

    trie_tree*                                  parser_list;
    std::unordered_map<std::string_view, int>   keywords;   //  name : TOKEN_T

    int token_2_reg(const token& tok);
    int operator_level(int op) const;
};
