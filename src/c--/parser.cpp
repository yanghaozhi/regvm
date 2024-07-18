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

#include "statements.h"


parser::parser() : parser_list(new trie_tree())
{
    keywords.emplace("if", If);
    keywords.emplace("else", Else);
    keywords.emplace("int", Int);
    keywords.emplace("double", Double);
    keywords.emplace("for", For);
    keywords.emplace("do", Do);
    keywords.emplace("while", While);
    keywords.emplace("break", Break);
    keywords.emplace("continue", Continue);
}


parser::~parser()
{
    //TODO
}

bool parser::go(const char* src, std::deque<inst>& out)
{
    LOGD("%s", src);

    decl_var_only       dvo(this);
    decl_var_init       dvi(this);
    call_func_no_ret    cfnr(this);
    assign_var          avar(this);
    if_else             ie(this);
    do_while            dw(this);
    while_loop          wl(this);
    for_loop            fl(this);

    while ((src != NULL) && (*src != '\0'))
    {
        src = statement(src);
    }

    out.swap(insts);
    return (src != NULL) ? true : false;
}

const char* parser::statement(const char* src, std::function<void (const token&)> cb)
{
    token tok;
    const char* p = src;
    src = next_token(src, tok);
    switch (tok.info.type)
    {
    case '}':
        INST(BLOCK, 0, 1);
        regs.cleanup(true);
        [[fallthrough]];
    case 0:
    case ';':
        return src;
    case '{':
        INST(BLOCK, 0, 0);
        regs.cleanup(true);
        while ((src != NULL) && (*src != '\0') && (*(src - 1) != '}'))
        {
            src = statement(src);
        }
        return src;
    case Break:
    case Continue:
        if (cb)
        {
            cb(tok);
            return src;
        }
        else
        {
            LOGE("Do NOT allow use break/continue outside loops !!!");
            return NULL;
        }
    default:
        src = p;
        break;
    }

    token toks[depth];

    trie_tree* cur = parser_list;
    int idx = 0;
    while ((src != NULL) && (*src != '\0'))
    {
        if (cur->next.empty() == true)
        {
            return cur->func->go(src, toks, idx);
        }

        if (idx >= depth)
        {
            LOGE("%d : too deep to find next token %d !!!", lineno, idx);
            return NULL;
        }

        token& tok = toks[idx];
        src = next_token(src, tok);
        switch (tok.info.type)
        {
        case 0:
        case ';':
            continue;
        }

        LOGD("%d %c", tok.info.type, (char)tok.info.orig);
        auto it = cur->next.find(tok.info.type);
        if (it == cur->next.end())
        {
            LOGE("%d : no parser op want token %d - %c : %s !!!", lineno, tok.info.type, (char)tok.info.orig, std::string(tok.name).c_str());
            return NULL;
        }
        cur = it->second;

        idx += 1;
    }
    return src;
}

bool parser::add(op* func, ...)
{
    trie_tree* cur = parser_list;

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
            cur = it->second;
        }
        else
        {
            cur = cur->next.emplace(t, new trie_tree()).first->second;
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

int parser::operator_level(int op) const
{
    //ref : https://en.cppreference.com/w/cpp/language/operator_precedence
    switch (op)
    {
    case Add:
    case Sub:
        return 6;
    case Mul:
    case Div:
    case Mod:
        return 5;
    case Shl:
    case Shr:
        return 7;
    case Gt:
    case Ge:
    case Lt:
    case Le:
        return 9;
    case Eq:
        return 10;
    //case Xor:
    //    return 12;
    //case Or:
    //    return 13;
    default:
        return -1;
    }
}

select::reg parser::token_2_reg(const token& tok)
{
    if (tok.reg.valid() == true)
    {
        return tok.reg;
    }

    switch (tok.info.type)
    {
    case Num:
        {
            auto reg = regs.tmp();
            INST(SETS, reg, tok.info.data_type, tok.info.value);
            return reg;
        }
        break;
    case Id:
        {
            //int n = regs.get();
            //INST(SETC, n, tok.name);
            //auto reg = regs.var(tok.name);
            //INST(LOAD, reg, n);
            std::string_view name = tok.name;
            auto reg = regs.get(tok.name, [this, name]()
                {
                    auto n = regs.tmp();
                    INST(SETC, n, name);
                    auto reg = regs.var(name);
                    INST(LOAD, reg, n);
                    return reg;
                });
            //INST(LOAD, reg.get([](select::reg* r)
            //        {
            //            auto n = regs.get();
            //            INST(SETC, n, tok.name);
            //            *id = n;
            //            *ver = 
            //        }), n);
            return reg;
        }
        break;
    default:
        LOGE("%d : invalid expression %d - %s !!!", lineno, tok.info.type, std::string(tok.name).c_str());
        break;
    }
    return select::reg();
}

template <typename T, typename O> select::reg parser::immediate_optimize(T& toks, O& ops)
{
    auto& tok = toks.back();
    if ((tok.info.type != Num) || (ops.size() == 0))
    {
        return token_2_reg(tok);
    }

    unsigned int v = tok.info.value.uint;

    switch (ops.back())
    {
#define OPTIMIZE(k, op, vv, ...)                                \
    case k:                                                     \
        if (v op vv)                                            \
        {                                                       \
            toks.pop_back();                                    \
            ops.pop_back();                                     \
            select::reg l = token_2_reg(toks.back());           \
            INST(__VA_ARGS__);                                  \
            return l;                                           \
        }                                                       \
        break;
        OPTIMIZE(Add, <=, 16, INC, l, v);
        OPTIMIZE(Sub, <=, 16, DEC, l, v);
        OPTIMIZE(Eq, ==, 0, CMP, l, 0);
        OPTIMIZE(Ne, ==, 0, CMP, l, 1);
        OPTIMIZE(Gt, ==, 0, CMP, l, 2);
        OPTIMIZE(Ge, ==, 0, CMP, l, 3);
        OPTIMIZE(Lt, ==, 0, CMP, l, 4);
        OPTIMIZE(Le, ==, 0, CMP, l, 5);
#undef OPTIMIZE
    default:
        break;
    }
    return token_2_reg(tok);
}

template <typename T, typename O> select::reg parser::pop_and_calc(T& toks, O& ops)
{
    if (toks.size() == 0)
    {
        LOGE("toks is empty");
        return select::reg();
    }
    if (ops.size() != toks.size() - 1)
    {
        LOGE("size of ops(%zd) or size of toks(%zd) invalid !!!", ops.size(), toks.size());
        return select::reg();
    }

    auto r = immediate_optimize(toks, ops);
    toks.pop_back();
    if (ops.size() == 0)
    {
        return r;
    }

    int op = -1;
    select::reg l;
    const int level = operator_level(ops.back());
    do
    {
        op = ops.back();
        ops.pop_back();
        l = token_2_reg(toks.back());
        toks.pop_back();

        switch (op)
        {
        case Add:
            INST(ADD, l, r);
            break;
        case Sub:
            INST(SUB, l, r);
            break;
        case Mul:
            INST(MUL, l, r);
            break;
        case Div:
            INST(DIV, l, r);
            break;
        case Mod:
            INST(MOD, l, r);
            break;
        case Eq:
            INST(SUB, l, r);
            INST(CMP, l, 0);
            break;
        case Ne:
            INST(SUB, l, r);
            INST(CMP, l, 1);
            break;
        case Gt:
            INST(SUB, l, r);
            INST(CMP, l, 2);
            break;
        case Ge:
            INST(SUB, l, r);
            INST(CMP, l, 3);
            break;
        case Lt:
            INST(SUB, l, r);
            INST(CMP, l, 4);
            break;
        case Le:
            INST(SUB, l, r);
            INST(CMP, l, 5);
            break;
        default:
            LOGE("%d : UNKNOWN operator of expression %d - %c !!!", lineno, op, (char)op);
            return select::reg();
        }
        r = l;
    } while ((ops.size() > 0) && (level <= operator_level(ops.back())));
    return l;
}

const char* parser::call_func(const char* src, const token& name, std::vector<select::reg>& rets)
{
    if (name.info.type != Id)
    {
        LOGE("%d : invalid function name %d - %s !!!", lineno, name.info.type, std::string(name.name).c_str());
        return NULL;
    }
    //TODO : need to check func name valid !!!
    if (name.name == "echo")
    {
        std::vector<select::reg> args;
        src = comma(src, args);
        if (src == NULL)
        {
            LOGE("invalid of comma expression !!!");
            return NULL;
        }

        std::vector<int> a;
        a.push_back(args.size());
        for (auto& it : args)
        {
            a.emplace_back((int)it);
        }
        rets.emplace_back(regs.lock());
        INST(CMD, (int)(rets[0]), 0, a);
    }
    return src;
}

const char* parser::expression(const char* src, select::reg& reg, int* end)
{
    std::deque<token>   toks;
    std::deque<int>     ops;

    while ((src != NULL) && (*src != '\0'))
    {
        if (toks.size() == ops.size())
        {
            auto& v = toks.emplace_back();
            src = next_token(src, v);
            switch (v.info.type)
            {
            case '(':
                src = expression(src, v.reg, end);
                break;
            case Num:
            case Id:
                break;
            case ';':
                toks.pop_back();
                reg = pop_and_calc(toks, ops);
                if (end != NULL)
                {
                    *end = ';';
                }
                return src;
            default:
                LOGE("%d : invalid token of expression %d - %c - %s !!!", lineno, v.info.type, v.info.orig, std::string(v.name).c_str());
                return NULL;
            }
        }

        token op;
        src = next_token(src, op);
        switch (op.info.type)
        {
        case ':':
        case ';':
        case ',':
        case ')':
            reg = pop_and_calc(toks, ops);
            if (end != NULL)
            {
                *end = op.info.type;
            }
            return src;
        case '(':   //函数调用
            {
                std::vector<select::reg> rets;
                src = call_func(src, toks.back(), rets);
                if (rets.size() >= 1)
                {
                    toks.back().reg = rets[0];
                }
            }
            break;
        default:
            {
                if (ops.size() > 0)
                {
                    int cur = operator_level(op.info.type);
                    int prev = operator_level(ops.back());
                    if (cur > prev)
                    {
                        auto rr = pop_and_calc(toks, ops);
                        auto& vv = toks.emplace_back();
                        vv.reg = rr;
                    }
                }
                ops.emplace_back(op.info.type);
            }
            break;
        }
        //if (r.valid() == true)
        //{
        //    auto& vv = toks.emplace_back();
        //    vv.reg = r;
        //}
        //else
        //{
        //    LOGW("expr : %d : %s", (int)r, src);
        //    reg.clear();
        //}
    }
    if (end != NULL)
    {
        *end = 0;
    }
    return src;
}

const char* parser::comma(const char* src, std::vector<select::reg>& rets)
{
    while ((src != NULL) && (*src != '\0'))
    {
        select::reg reg; 
        int end = -1;
        src = expression(src, reg, &end);
        if (reg < 0)
        {
            LOGE("invalid expression result : %d : %s !!!", (int)reg, src);
            return NULL;
        }
        rets.emplace_back(reg);

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
            LOGE("%d : comma expression should NOT end with %d - %c\n", lineno, end, (char)end);
            return NULL;
        }
    }
    return src;
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

template <typename F, typename ... Args> const char* whole_number(const char* str, uv& ret, DATA_TYPE& type, DATA_TYPE ok, F func, Args ... args)
{
    char* end = NULL;
    ret.uint = func(str, &end, args ...);
    if (*end != '.')
    {
        type = ok;
    }
    else
    {
        ret.dbl = strtod(str, &end);
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
            lineno += 1;
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
                    return whole_number(next - 1, tok.info.value, tok.info.data_type, TYPE_DOUBLE, strtod);
                default:
                    base = 8;
                    break;
                };
                tok.info.type = Num;
                return whole_number(next - 1, tok.info.value, tok.info.data_type, TYPE_UNSIGNED, strtoull, base);
            }
        case '1' ... '9':   //十进制
            tok.info.type = Num;
            src = whole_number(next - 1, tok.info.value, tok.info.data_type, TYPE_UNSIGNED, strtoull, 10);
            if (tok.info.value.uint < 0x7FFFFFFFFFFFFFFF)
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
                return whole_number(next - 1, tok.info.value, tok.info.data_type, TYPE_SIGNED, strtoll, 10);   \
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

