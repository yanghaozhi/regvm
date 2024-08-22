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

class func;

class parser
{
public:
    parser();
    ~parser();

    class op
    {
    public:
        insts_t*        insts       = NULL;
        func*           f           = NULL;

        op(parser* p)   {};
        virtual const char* go(const char* src, const token* toks, int count)    = 0;
    };


    bool go(const char* src, insts_t& insts);

    bool add(op* func, ...);

    const char* next_token(const char* src, token& tok);

    const char* find_statement(const char* src, func* f);

private:
    struct trie_tree
    {
        op*                         func        = NULL;
        std::map<int, trie_tree>    next;
    };

    int                             depth       = 0;
    selector                        regs;
    trie_tree                       parser_list;
    std::unordered_map<std::string_view, int>   keywords;   //  name : TOKEN_T
    
    const char* find_statement(const char* src, func* f, trie_tree& cur, token* toks, int idx, int max);
    const char* statement(const char* src, std::function<void (const token&)> cb, token& tok);
};

