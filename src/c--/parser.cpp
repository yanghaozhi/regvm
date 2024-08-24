#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <deque>
#include <string>
#include <typeinfo>

#include <log.h>
#include <code.h>

#include "func.h"
#include "statements.h"



parser::parser() : regs()
{
    keywords.emplace("if", If);
    keywords.emplace("else", Else);
    keywords.emplace("register", Register);
    keywords.emplace("int", Int);
    keywords.emplace("double", Double);
    keywords.emplace("for", For);
    keywords.emplace("do", Do);
    keywords.emplace("while", While);
    keywords.emplace("break", Break);
    keywords.emplace("continue", Continue);
    keywords.emplace("return", Return);

    cmds.emplace("echo", cmd_echo);
}


parser::~parser()
{
}

bool parser::go(const char* f, const char* src, insts_t& out)
{
    file = f;

    decl_var_only       dvo(this);
    decl_var_init       dvi(this);
    call_func_no_ret    cfnr(this);
    assign_var          avar(this);
    assign_equal        aeq(this);
    if_else             ie(this);
    do_while            dw(this);
    while_loop          wl(this);
    for_loop            fl(this);
    decl_func           df(this);
    ret_func            rf(this);

    find_line_ending(src);

    func entry(this);

    src = entry.go(src);
    if (src == NULL)
    {
        return false;
    }
    out.swap(*entry.insts);
    return true;
}

const char* parser::find_statement(const char* src, func* f)
{
    token toks[depth];

    return find_statement(src, f, parser_list, toks, 0, depth);
}

const char* parser::find_statement(const char* src, func* f, trie_tree& cur, token* toks, int idx, int max)
{
    if ((src != NULL) && (*src != '\0'))
    {
        if (cur.next.empty() == true)
        {
            cur.func->f = f;
            cur.func->insts = f->insts;
            return cur.func->go(src, toks, idx);
        }

        if (idx >= max)
        {
            //LOGE("%d : too deep to find next token %d !!!", lineno, idx);
            return NULL;
        }

        token& tok = toks[idx];
        src = next_token(src, tok);
        if (tok.info.type == 0)
        {
            return NULL;
        }

        auto it = cur.next.find(tok.info.type);
        if (it == cur.next.end())
        {
            //LOGW("%d : no parser op want token %d - %c : %s !!!", lineno, tok.info.type, (char)tok.info.orig, std::string(tok.name).c_str());
            return (cur.func != NULL) ? cur.func->go(src, toks, idx) : NULL;
        }

        const char* r = find_statement(src, f, it->second, toks, idx + 1, max);
        if ((r == NULL) && (cur.func != NULL))
        {
            cur.func->f = f;
            cur.func->insts = f->insts;
            r = cur.func->go(src, toks, idx);
        }
        return r;
    }
    return NULL;
}

bool parser::add(op* func, ...)
{
    trie_tree* cur = &parser_list;

    LOGT("add %s ", typeid(*func).name());

    va_list ap;
    va_start(ap, func);
    int t = -1;
    int d = 0;
    while ((t = va_arg(ap, int)) >= 0)
    {
        auto it = cur->next.find(t);
        if (it != cur->next.end())
        {
            cur = &it->second;
        }
        else
        {
            cur = &cur->next.emplace(t, trie_tree{}).first->second;
        }
        d += 1;
    }
    va_end(ap);

    cur->func = func;
    if (d > depth)
    {
        depth = d;
    }

    return true;
}

inline uint64_t whole_token(const char* pos, const char** end)
{
    uint64_t h = 0;
    while((*pos >= 'a' && *pos <= 'z') || (*pos >= 'A' && *pos <= 'Z') || (*pos >= '0' && *pos <= '9') || *pos == '_')
    {
        h = h * 131 + *pos++;
    }
    *end = pos;
    return h;
}

template <typename T, typename F, typename ... Args> const char* whole_number(const char* str, T& ret, double& dbl, DATA_TYPE& type, DATA_TYPE ok, F func, Args ... args)
{
    char* end = NULL;
    ret = func(str, &end, args ...);
    if (*end != '.')
    {
        type = ok;
    }
    else
    {
        dbl = strtod(str, &end);
        type = TYPE_DOUBLE;
    }
    return end;
}

const char* parser::next_token(const char* src, token& tok)
{
    const char* next = src;
    memset(&tok.info, 0, sizeof(tok.info));
    tok.name = "";

    while ((next != NULL) && (*next != 0))
    {
        const char* end = NULL;
        int token = *next++;
        tok.info.orig = token;
        switch (token)
        {
        case ' ':
            continue;
        case '#':   //不支持宏
            next = strchr(next, '\n');
            continue;
        case '\n':
            find_line_ending(next);
            break;
        case '\'':  //字符
            tok.info.type = Num;
            break;
        case '"':  //字符串
            //TODO : 暂不支持转义字符
            end = strchr(next, token);
            tok.info.data_type = TYPE_STRING;
            tok.info.type = token;
            tok.name = std::string_view(next - 1, end - next + 1);
            return end + 1;
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':   //解析合法的变量名
            {
                whole_token(next - 1, &end);
                tok.name = std::string_view(next - 1, end - next + 1);
                auto it = keywords.find(tok.name);
                tok.info.type = (it != keywords.end()) ? it->second : Id;
                return end;
            }
        case '0':   //解析数字(十六进制,八进制)
            {
                int base = 10;
                switch (*next)
                {
                case 'x':
                case 'X':
                    base = 16;
                    break;
                case '.':
                    tok.info.type = Num;
                    return whole_number(next - 1, tok.info.value.dbl, tok.info.value.dbl, tok.info.data_type, TYPE_DOUBLE, strtod);
                default:
                    base = 8;
                    break;
                };
                tok.info.type = Num;
                return whole_number(next - 1, tok.info.value.uint, tok.info.value.dbl, tok.info.data_type, TYPE_UNSIGNED, strtoull, base);
            }
        case '1' ... '9':   //十进制
            tok.info.type = Num;
            src = whole_number(next - 1, tok.info.value.uint, tok.info.value.dbl, tok.info.data_type, TYPE_UNSIGNED, strtoull, 10);
            if ((tok.info.data_type == TYPE_UNSIGNED) && (tok.info.value.uint < 0x7FFFFFFFFFFFFFFF))
            {
                tok.info.data_type = TYPE_SIGNED;
            }
            return src;

#define COMBINE_OP(TOK, DEF, EQ, EXTRA, ...)                \
        case TOK:                                           \
            switch (*next)                                  \
            {                                               \
            case '=':                                       \
                tok.info.type = EQ;                         \
                return next + 1;                            \
                break;                                      \
            EXTRA(TOK, ##__VA_ARGS__);                      \
            default:                                        \
                tok.info.type = DEF;                        \
                return next;                                \
            }                                               \
            break;

//TODO : 不支持多行注释
#define COMMENT(TOK)                                        \
            case TOK:                                       \
                next = strchr(next, '\n');                  \
                continue;
#define DUP(TOK, TYPE)                                      \
            case TOK:                                       \
                tok.info.type = TYPE;                       \
                return next + 1;
#define NUM(TOK, TYPE)                                      \
            case '0' ... '9':                               \
                tok.info.type = Num;                        \
                return whole_number(next - 1, tok.info.value.sint, tok.info.value.dbl, tok.info.data_type, TYPE_SIGNED, strtoll, 10);   \
            DUP(TOK, TYPE)
#define NOP(...)

            COMBINE_OP('+', Add, AddE, NUM, Inc);
            COMBINE_OP('-', Sub, SubE, NUM, Dec);
            COMBINE_OP('*', Mul, MulE, NOP);
            COMBINE_OP('/', Div, DivE, COMMENT);
            COMBINE_OP('%', Mod, ModE, NOP);
            COMBINE_OP('>', Gt, Ge, DUP, Shr);
            COMBINE_OP('<', Lt, Le, DUP, Shl);
            COMBINE_OP('=', Assign, Eq, NOP);
            COMBINE_OP('!', Not, Ne, NOP);

#undef NOP
#undef DUP
#undef NUM 
#undef COMMENT

#undef COMBINE_OP

        case '~': 
        case ';': 
        case '{': 
        case '}': 
        case '(': 
        case ')': 
        case '[': 
        case ']': 
        case ',': 
        case ':':
            tok.info.type = token;
            return next;
        default:
            break;
        }
    }
    return next;
}

void parser::find_line_ending(const char* src)
{
    if ((line_end != NULL) && (src <= line_end)) return;

    lineno += 1;
    line_end = strchr(src, '\n');
    if (line_end != NULL)
    {
        line = std::string_view(src, line_end - src);
    }
    else
    {
        line = std::string_view(src);
        line_end = src + strlen(src);
    }
    LOGD("%d : %s", lineno, VIEW(line));
}

void parser::show_error(const char* fmt, ...)
{
    fprintf(stderr, "\e[1;35m %s : %d ", file.c_str(), lineno);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n\t%s \e[0m\n", VIEW(line));
}

const char* parser::cmd_echo(const char* src, func* f, const token& name, std::vector<selector::reg>& rets)
{
    insts_t* insts = f->insts;
    std::vector<int> args;
    src = f->comma(src, [&args, f](const char* src, int* end)
        {
            selector::reg reg; 
            src = f->expression(src, reg, end, NULL);
            if (reg.ptr == NULL)
            {
                LOGE("invalid expression result : %d:%p : %s !!!", reg.ver, reg.ptr, src);
            }
            args.emplace_back((int)reg);
            return src;
        });
    INST(ECHO, args);
    return src;
}

