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
            fprintf(stderr, "%d : no parser op want token %s !!!\n", lineno, std::string(tok.name).c_str());
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

const char* parser::expression(const char* src, int reg, DATA_TYPE type)
{
    token tok[3];
    src = next_token(src, tok[0]);
    src = next_token(src, tok[1]);
    if (tok[1].info.type == ';')
    {
        switch (tok[0].info.type)
        {
        case Num:
            INST(SETS, reg, type, tok[0].info.value);
            break;
        case Id:
            {
                int n = regs.get();
                INST(SETC, n, tok[0].name);
                INST(LOAD, reg, n);
            }
            break;
        default:
            fprintf(stderr, "%d : invalid expression %d - %s !!!\n", lineno, tok[0].info.type, std::string(tok[0].name).c_str());
            return NULL;
        }
        return src;
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

    while (*next != 0)
    {
        const char* end = NULL;
        int token = *next++;
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
            tok.info.value.uint = token;
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
            break;
        case '*':
            tok.info.type = Mul;
            break;
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

