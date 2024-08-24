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
#include "func.h"
#include "inst.h"

#define COMPILE_ERROR(p, fmt, ...)  p->show_error(" | %s:%d | " fmt, __FILE__, __LINE__, ##__VA_ARGS__);

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
        parser*         p           = NULL;

        op(parser* pp) : p(pp)      {};
        virtual const char* go(const char* src, const token* toks, int count)    = 0;
    };


    bool go(const char* file, const char* src, insts_t& insts);

    bool add(op* func, ...);

    const char* next_token(const char* src, token& tok);

    const char* find_statement(const char* src, func* f);

    void show_error(const char* fmt, ...);

    int32_t                         func_id     = 0;
    std::unordered_map<std::string_view, func>  funcs;

    typedef const char* (*cmd_t)(const char* src, func* f, const token& name, std::vector<selector::reg>& rets);
    std::unordered_map<std::string_view, cmd_t> cmds;

private:
    friend class func;

    struct trie_tree
    {
        op*                         func        = NULL;
        std::map<int, trie_tree>    next;
    };

    int                             lineno      = 0;
    const char*                     line_end    = NULL;
    std::string_view                line;
    std::string                     file;
    int                             depth       = 0;
    selector                        regs;
    trie_tree                       parser_list;
    std::unordered_map<std::string_view, int>   keywords;   //  name : TOKEN_T
    
    const char* find_statement(const char* src, func* f, trie_tree& cur, token* toks, int idx, int max);
    const char* statement(const char* src, std::function<void (const token&)> cb, token& tok);

    void find_line_ending(const char* src);

    static const char* cmd_echo(const char* src, func* f, const token& name, std::vector<selector::reg>& rets);
};

