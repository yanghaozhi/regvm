#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

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
    int reg = regs.get();
    switch (tok.info.type)
    {
    case Num:
        INST(SETS, reg, tok.info.data_type, tok.info.value);
        break;
    case Id:
        {
            int n = regs.get();
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

const char* parser::expression(const char* src, const int prev_level, int& reg)
{
    token tok[2];
    src = next_token(src, tok[0]);
    switch (tok[0].info.type)
    {
    case '(':
        src = expression(src, -1, reg);
        if ((src == NULL) || (reg < 0))
        {
            return NULL;
        }
        break;
    case Num:
    case Id:
        break;
    default:
        fprintf(stderr, "%d : invalid token of expression %d - %c - %s !!!\n", lineno, tok[0].info.type, (char)tok[0].info.type, std::string(tok[0].name).c_str());
        return NULL;
    }

    do
    {
        src = next_token(src, tok[1]);
        int level = operator_level(tok[1].info.type);
        if ((prev_level >= 0) && (prev_level < level))
        {
            return src - 1;
        }
        switch (tok[1].info.type)
        {
        case ';':
            reg = token_2_reg(tok[0]);
            printf("$%d = %ld\n", reg, tok[0].info.value.sint);
            return (reg >= 0) ? src - 1 : NULL;
        case ')':
            reg = token_2_reg(tok[0]);
            printf("$%d = %ld\n", reg, tok[0].info.value.sint);
            return (reg >= 0) ? src : NULL;
        default:
            if ((prev_level >= 0) && (level > prev_level))
            {
                reg = token_2_reg(tok[0]);
                printf("$%d = %ld\n", reg, tok[0].info.value.sint);
                return (reg >= 0) ? src - 1 : NULL;
            }
            break;
        }

        int n = -1;
        src = expression(src, level, n);
        if ((src == NULL) || (n < 0))
        {
            return NULL;
        }

        if (reg < 0)
        {
            reg = token_2_reg(tok[0]);
            if (reg < 0)
            {
                return NULL;
            }
            printf("$%d = %ld %c $%d\n", reg, tok[0].info.value.sint, (char)tok[1].info.orig, n);
        }
        else
        {
            printf("$%d = $%d %c $%d\n", reg, reg, (char)tok[1].info.orig, n);
        }

        switch (tok[1].info.type)
        {
        case Add:
            INST(ADD, reg, n);
            break;
        case Sub:
            INST(SUB, reg, n);
            break;
        case Mul:
            INST(MUL, reg, n);
            break;
        case Div:
            INST(DIV, reg, n);
            break;
        default:
            fprintf(stderr, "%d : UNKNOWN operator of expression %d - %s !!!\n", lineno, tok[1].info.type, std::string(tok[1].name).c_str());
            return NULL;
        }
    } while (0);

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

    while (*next != 0)
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
                tok.info.type = Add;
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

