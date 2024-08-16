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




parser::parser() : regs(), scopes(insts, regs), parser_list(new trie_tree())
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
}


parser::~parser()
{
    //TODO
}

bool parser::go(const char* src, insts_t& out)
{
    LOGD("%s", src);

    last_line = src;
    lineno = 0;

    decl_var_only       dvo(this);
    decl_var_init       dvi(this);
    call_func_no_ret    cfnr(this);
    assign_var          avar(this);
    assign_equal        aeq(this);
    if_else             ie(this);
    do_while            dw(this);
    while_loop          wl(this);
    for_loop            fl(this);

    scopes.enter();
    while ((src != NULL) && (*src != '\0'))
    {
        src = statement(src);
    }
    scopes.leave();

    out.swap(insts);
    return (src != NULL) ? true : false;
}

const char* parser::statements(const char* src, std::function<void (const token&)> cb)
{
    token tok;
    const char* p = next_token(src, tok);
    switch (tok.info.type)
    {
    case '{':
        scopes.enter();
        //INST(BLOCK, 0, 0, 0);
        while ((p != NULL) && (tok.info.type != '}'))
        {
            p = statement(p, cb, tok);
        }
        //INST(BLOCK, 0, 1, 0);
        scopes.leave();
        return p;
    case 0:
    case ';':
        return p;
    default:
        return statement(src, cb, tok);
    }
}

const char* parser::statement(const char* src, std::function<void (const token&)> cb, token& tok)
{
    const char* p = src;
    src = next_token(src, tok);
    switch (tok.info.type)
    {
    case 0:
    case ';':
        return src;
    case '{':
        return statements(src, cb);
    case '}':
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

    src = find_statement(src, parser_list, toks, 0, depth);
    if (src == NULL)
    {
        LOGE("%d : no valid parser op !!!", lineno);
        return NULL;
    }
    return src;
}

const char* parser::statement(const char* src)
{
    token tok;
    return statement(src, NULL, tok);
}

const char* parser::find_statement(const char* src, trie_tree* cur, token* toks, int idx, int max)
{
    if ((src != NULL) && (*src != '\0'))
    {
        if (cur->next.empty() == true)
        {
            return cur->func->go(src, toks, idx);
        }

        if (idx >= max)
        {
            LOGE("%d : too deep to find next token %d !!!", lineno, idx);
            return NULL;
        }

        token& tok = toks[idx];
        src = next_token(src, tok);
        if (tok.info.type == 0)
        {
            return NULL;
        }

        auto it = cur->next.find(tok.info.type);
        if (it == cur->next.end())
        {
            LOGW("%d : no parser op want token %d - %c : %s !!!", lineno, tok.info.type, (char)tok.info.orig, std::string(tok.name).c_str());
            return (cur->func != NULL) ? cur->func->go(src, toks, idx) : NULL;
        }

        const char* r = find_statement(src, it->second, toks, idx + 1, max);
        if ((r == NULL) && (cur->func != NULL))
        {
            r = cur->func->go(src, toks, idx);
        }
        return r;
    }
    return NULL;
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

inline int calc_op(int op)
{
    switch (op)
    {
#define CALC(k, op)         \
    case k:                 \
        return op;
        CALC(Add, 0);
        CALC(Sub, 1);
        CALC(Mul, 2);
        CALC(Div, 3);
        CALC(Mod, 4);
        CALC(Shl, 5);
        CALC(Shr, 6);
#undef CALC
    default:
        return -10000;
    }
}

int cmp_op(int op)
{
    switch (op)
    {
#define CALC(k, op)         \
    case k:                 \
        return op;
        CALC(Eq, 0);
        CALC(Ne, 1);
        CALC(Gt, 2);
        CALC(Ge, 3);
        CALC(Lt, 4);
        CALC(Le, 5);
#undef CALC
    default:
        return -10000;
    }
}

inline int operator_level(int op)
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

selector::reg parser::token_2_reg(const token& tok)
{
    if (regs.active(tok.reg) == true)
    {
        return tok.reg;
    }

    switch (tok.info.type)
    {
    case Num:
        {
            auto reg = regs.tmp();
            switch (tok.info.data_type)
            {
            case TYPE_SIGNED:
                INST(SET, reg, tok.info.value.sint);
                break;
            case TYPE_UNSIGNED:
                INST(SET, reg, tok.info.value.uint);
                break;
            case TYPE_DOUBLE:
                INST(SET, reg, tok.info.value.dbl);
                break;
            default:
                return selector::reg();
            }
            return reg;
        }
        break;
    case Id:
        {
            std::string_view name = tok.name;
            int attr = 0;
            auto v = scopes.find_var(tok.name, attr, [this, name](int attr, uint64_t id)
                {
                    auto n = regs.tmp();
                    INST(SET, n, TYPE_ADDR, id);
                    auto v = scopes.new_var(name, 0);
                    INST(LOAD, v->reg, n, 0);
                    return v->reg;
                });
            return v->reg;
        }
        break;
    default:
        LOGE("%d : invalid expression %d - %s !!!", lineno, tok.info.type, std::string(tok.name).c_str());
        break;
    }
    return selector::reg();
}

bool can_literally_optimize(const token& a, int& v)
{
    if (a.info.type != Num)
    {
        return false;
    }

    switch (a.info.data_type)
    {
    case TYPE_SIGNED:
        v = a.info.value.sint;
        if ((v < -127) || (v > 127)) return false;
        break;
    case TYPE_UNSIGNED:
        v = a.info.value.uint;
        if (v > 127) return false;
        break;
    default:
        return false;
    }
    return true;
}

inline bool literally_calc(parser* p, int op, const token& a, const selector::reg& b)
{
    switch (b.ptr->status)
    {
    case selector::BINDED:
    case selector::LOCKED:
        return false;
    default:
        break;
    }

    int v = 0;
    if (can_literally_optimize(a, v) == false)
    {
        return false;
    }

    auto& insts = p->insts;
    switch (op)
    {
    case Add:
    case Mul:
        INST(CALC, b, v, calc_op(op));
        return true;
    default:
        return false;
    }
}

inline bool literally_cmp(parser* p, int op, const token& a, const selector::reg& b)
{
    return false;
}

inline selector::reg calc_a_b(parser* p, int op, const token& a, const selector::reg& b)
{
    auto& insts = p->insts;

    if ((300 <= op) && (op < 320))
    {
        if (literally_calc(p, op, a, b) == true)
        {
            return b;
        }
    }

    selector::reg r = p->regs.tmp();

    auto c = p->token_2_reg(a);
    switch (op)
    {
    case Add:
        INST(ADD, r, c, b);
        break;
    case Sub:
        INST(SUB, r, c, b);
        break;
    case Mul:
        INST(MUL, r, c, b);
        break;
    case Div:
        INST(DIV, r, c, b);
        break;
    case Mod:
        INST(MOD, r, c, b);
        break;
    case Eq:
    case Ne:
    case Gt:
    case Ge:
    case Lt:
    case Le:
        INST(SUB, r, c, b);
        INST(CMP, r, r, cmp_op(op));
        break;
    default:
        LOGE("%d : UNKNOWN operator of expression %d - %c !!!", p->lineno, op, (char)op);
        return selector::reg();
    }
    return r;
}

inline selector::reg calc_a_b(parser* p, int op, const token& a, const token& b)
{
    int v = 0;
    if (can_literally_optimize(b, v) == true)
    {
        auto& insts = p->insts;
        auto v1 = p->token_2_reg(a);
        switch (op)
        {
        case Add:
        case Sub:
        case Mul:
        case Div:
        case Mod:
            switch (v1.ptr->status)
            {
            case selector::BINDED:
            case selector::LOCKED:
                break;
            default:
                INST(CALC, v1, v, calc_op(op));
                return v1;
            }
            break;
        default:
            break;
        };
    }
    auto v2 = p->token_2_reg(b);
    if (can_literally_optimize(a, v) == true)
    {
        if (literally_calc(p, op, a, v2) == true)
        {
            return v2;
        }
    }
    return calc_a_b(p, op, a, v2);
}

template <typename T, typename O> selector::reg pop_and_calc(parser* p, T& toks, O& ops, const parser::calc_t& calc)
{
    if (toks.size() == 0)
    {
        LOGE("toks is empty");
        return selector::reg();
    }
    if (ops.size() == 0)
    {
        auto r = p->token_2_reg(toks.back());
        toks.pop_back();
        return r;
    }
    if (ops.size() != toks.size() - 1)
    {
        LOGE("size of ops(%zd) or size of toks(%zd) invalid !!!", ops.size(), toks.size());
        return selector::reg();
    }

    const auto& v2 = toks.back();
    selector::reg b;
    int level = -1;
    if (v2.info.type == Num)
    {
        int op = ops.back();
        const auto& v1 = toks[toks.size() - 2];
        if ((calc != NULL) && (ops.size() == 1))
        {
            auto r = calc(p, op, v1, &v2, NULL);
            if (r.ptr != NULL)
            {
                toks.clear();
                ops.clear();
                return r;
            }
        }
        b = calc_a_b(p, op, v1, v2);
        level = operator_level(op);
        toks.pop_back();
        ops.pop_back();
    }
    else
    {
        b = p->token_2_reg(v2);
    }
    toks.pop_back();

    while ((ops.size() > 0) && (level <= operator_level(ops.back())))
    {
        int op = ops.back();
        auto& a = toks.back();

        if ((calc != NULL) && (ops.size() == 1))
        {
            auto r = calc(p, op, a, NULL, &b);
            if (r.ptr != NULL)
            {
                toks.clear();
                ops.clear();
                return r;
            }
        }
        b = calc_a_b(p, op, a, b);

        toks.pop_back();
        ops.pop_back();
    }
    return b;
}

const char* parser::call_func(const char* src, const token& name, std::vector<selector::reg>& rets)
{
    if (name.info.type != Id)
    {
        LOGE("%d : invalid function name %d - %s !!!", lineno, name.info.type, std::string(name.name).c_str());
        return NULL;
    }
    //TODO : need to check func name valid !!!
    if (name.name == "echo")
    {
        std::vector<selector::reg> args;
        src = comma(src, args);
        if (src == NULL)
        {
            LOGE("invalid of comma expression !!!");
            return NULL;
        }

        std::vector<int> a;
        for (auto& it : args)
        {
            a.emplace_back((int)it);
        }
        INST(ECHO, a);
    }
    return src;
}

const char* parser::expression(const char* src, selector::reg& reg)
{
    int end = 0;
    return expression(src, reg, &end, NULL);
}

const char* parser::expression(const char* src, selector::reg& reg, int* end, const calc_t& calc)
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
                src = expression(src, v.reg, end, calc);
                break;
            case Num:
            case Id:
                break;
            case ';':
                toks.pop_back();
                reg = pop_and_calc(this, toks, ops, calc);
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
            reg = pop_and_calc(this, toks, ops, calc);
            if (end != NULL)
            {
                *end = op.info.type;
            }
            return src;
        case '(':   //函数调用
            {
                std::vector<selector::reg> rets;
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
                        auto rr = pop_and_calc(this, toks, ops, calc);
                        auto& vv = toks.emplace_back();
                        vv.reg = rr;
                    }
                }
                ops.emplace_back(op.info.type);
            }
            break;
        }
    }
    if (end != NULL)
    {
        *end = 0;
    }
    return src;
}

const char* parser::comma(const char* src, std::vector<selector::reg>& rets)
{
    while ((src != NULL) && (*src != '\0'))
    {
        selector::reg reg; 
        int end = -1;
        src = expression(src, reg, &end, NULL);
        if (reg.ptr == NULL)
        {
            LOGE("invalid expression result : %d:%p : %s !!!", reg.ver, reg.ptr, src);
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
            LOGD("%s", std::string(last_line, src - last_line).c_str());
            last_line = src;
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

