#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <deque>
#include <string>
#include <typeinfo>

#include <code.h>


parser::parser() : parser_list(new trie_tree())
{
    keywords.emplace("else", Else);
    keywords.emplace("if", If);
    keywords.emplace("int", Int);
    keywords.emplace("double", Double);
    keywords.emplace("return", Return);
    keywords.emplace("while", While);
}


parser::~parser()
{
    //TODO
}


const char* parser::statement(const char* src)
{
    token toks[depth];

    trie_tree* cur = parser_list;
    int idx = 0;
    while ((src != NULL) && (*src != '\0'))
    {
        if (cur->next.empty() == true)
        {
            return cur->func->go(this, src, toks, idx);
        }

        if (idx >= depth)
        {
            fprintf(stderr, "%d : too deep to find next token %d !!!\n", lineno, idx);
            return NULL;
        }

        token& tok = toks[idx];
        src = next_token(src, tok);
        if (tok.info.type == 0)
        {
            continue;
        }

        printf("%d\n", tok.info.type);
        auto it = cur->next.find(tok.info.type);
        if (it == cur->next.end())
        {
            fprintf(stderr, "%d : no parser op want token %c : %s !!!\n", lineno, (char)tok.info.type, std::string(tok.name).c_str());
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

    printf("add %s ", typeid(func).name());
    va_list ap;
    va_start(ap, func);
    int t = -1;
    int d = 0;
    while ((t = va_arg(ap, int)) >= 0)
    {
        printf("%d ", t);

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

    printf("\n");
    cur->func = func;
    if (d > depth)
    {
        depth = d;
    }

    return true;
}

int parser::operator_level(int op) const
{
    //ref : https://zh.cppreference.com/w/cpp/language/operator_precedence
    switch (op)
    {
    case Add:
    case Sub:
        return 6;
    case Mul:
    case Div:
        return 5;
    case Shl:
    case Shr:
        return 7;
    //case Xor:
    //    return 12;
    //case Or:
    //    return 13;
    default:
        return -1;
    }
}

int parser::token_2_reg(const token& tok)
{
    if (tok.reg >= 0)
    {
        return tok.reg;
    }

    int reg = regs.get();
    switch (tok.info.type)
    {
    case Num:
        INST(SETS, reg, tok.info.data_type, tok.info.value);
        break;
    case Id:
        {
            int n = regs.tmp();
            INST(SETC, n, tok.name);
            INST(LOAD, reg, n);
        }
        break;
    default:
        fprintf(stderr, "%d : invalid expression %d - %s !!!\n", lineno, tok.info.type, std::string(tok.name).c_str());
        return -1;
    }
    return reg;
}

template <typename T, typename O> int parser::pop_and_calc(T& toks, O& ops, const int level)
{
    int op = -1;
    int l = -1;
    if (ops.size() == 0)
    {
        return -1;
    }
    if (ops.size() != toks.size() - 1)
    {
        assert(0);
        return -1;
    }

    int r = token_2_reg(toks.back());
    toks.pop_back();
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
        default:
            //fprintf(stderr, "%d : UNKNOWN operator of expression %d - %s !!!\n", lineno, tok[1].info.type, std::string(tok[1].name).c_str());
            return -1;
        }
        r = l;
    } while ((ops.size() > 0) && ((level < 0) || (level == operator_level(ops.back()))));
    return l;
}

const char* parser::expression(const char* src, int& reg)
{
    std::deque<token>   toks;
    std::deque<int>     ops;

    while ((src != NULL) && (*src != '\0'))
    {
        auto& v = toks.emplace_back();
        src = next_token(src, v);
        switch (v.info.type)
        {
        case '(':
            src = expression(src, v.reg);
            break;
        case Num:
        case Id:
            break;
        default:
            fprintf(stderr, "%d : invalid token of expression %d - %c - %s !!!\n", lineno, v.info.type, v.info.orig, std::string(v.name).c_str());
            return NULL;
        }

        token op;
        src = next_token(src, op);
        int r = -1;
        switch (op.info.type)
        {
        case ';':
        case ')':
            reg = pop_and_calc(toks, ops, -1);
            return src;
        default:
            ops.emplace_back(op.info.type);
            if (operator_level(op.info.type) > operator_level(ops.back()))
            {
                r = pop_and_calc(toks, ops, operator_level(ops.back()));
            }
            break;
        }
        if (r >= 0)
        {
            auto& vv = toks.emplace_back();
            vv.reg = r;
        }
        else
        {
            reg = r;
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
        case '-':
            switch (*next)
            {
            case '-':
                tok.info.type = Dec;
                return next + 1;
            case '=':
                tok.info.type = SubE;
                return next + 1;
                break;
            case '0' ... '9':
                tok.info.type = Num;
                return whole_number(next - 1, tok.info.value, tok.info.data_type, TYPE_SIGNED, strtoll, 10);
            default:
                tok.info.type = Sub;
                return next;
            }
            break;
        case '+':
            switch (*next)
            {
            case '+':
                tok.info.type = Inc;
                return next + 1;
            case '=':
                tok.info.type = AddE;
                return next + 1;
            default:
                tok.info.type = Add;
                return next;
            }
        case '*':
            tok.info.type = Mul;
            return next;
        case '/':   //TODO : 不支持多行注释
            if (*next == '/')
            {
                next = strchr(next, '\n');
                continue;
            }
            else
            {
                tok.info.type = Div;
                return next;
            }
        case '=':
            if(*next != '=')
            {
                tok.info.type = Assign;
                return next;
            }
            else
            {
                tok.info.type = Eq;
                return next + 1;
            }
        case '~': 
        case ';': 
        case '{': 
        case '}': 
        case '(': 
        case ')': 
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

