/*
 * This module is rewrite from c4 project :https://github.com/rswier/c4
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <string>
#include <array>
#include <map>
#include <unordered_set>
#include <unordered_map>

//数据类型
enum DATA_T { UNKNOWN, STR, SINT, UINT, DOUBLE };

//支持的标记类别(供词法分析器next解析成对应的标记)
enum TOKEN_T { Num = 128,   //避免和ascii字符冲突
    Str, Fun, Sys, Glo, Loc, Id, Char, Else, Enum, If, Int, Return, Sizeof, While, Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak};


struct symbol
{
    union
    {
        int64_t             sint;
        uint64_t            uint;
        double              dbl;
        const char*         str;
        void*               ptr;
    }   value;

    int         len;
    uint64_t    hash;
    DATA_T      data_type;
    int         token_type;

    //int token       = 0;
    //int cls         = 0;
    //int type        = 0;
    //int tmp_cls     = 0;
    //int tmp_type    = 0;
    //int tmp_value   = 0;
};

class symtab
{
public:
    bool add(symbol& sym)
    {
        auto r = symbols.emplace(std::string(sym.value.str, sym.len), sym);
        printf("add symbol : %s\n", r.first->first.c_str());
        return r.second;
    };

private:
    //std::multimap<uint64_t, symbol> symbols;
    std::unordered_map<std::string, symbol> symbols;
};


//struct keyword
//{
//    int k;
//    int l;
//};
//const keyword TOKEN_CHAR[] = {{'a', 26}, {'_', 1}, {'A', 26}, {'0', 10}};
//const keyword NUMBER_CHAR[] = {{'a', 26}, {'_', 1}, {'A', 26}, {'0', 10}};

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

const char* find_token(const char* src, symbol* sym, int& lineno)
{
    const char* next = src;
    memset(sym, 0, sizeof(symbol));
    //char* begin_pos;
    //int hash;
    //while ((token = *pos++) != 0)
    while (*next != 0)
    {
        int base = 10;
        int token = *next++;
        switch (token)
        {
        case ' ':
            continue;
        case '#':   //由于不支持宏,所以直接跳过
            next = strchr(next, '\n');
            continue;
        case '\n':
            lineno += 1;
            break;
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':   //解析合法的变量名
            {
                const char* end = NULL;
                sym->hash = whole_token(next - 1, &end);
                sym->len = end - next + 1;
                sym->value.str = next - 1;
                sym->token_type = Id;
                ////token.name = std::string(next - 1, end);
                //symbols->add(next - 1, end - next + 1, h, sym);
                return end;
            }
        case '0':   //解析数字(十六进制,八进制)
            switch (*next)
            {
            case 'x':
            case 'X':
                base = 16;
                break;
            default:
                base = 8;
                break;
            };
            [[fallthrough]];
        case '1' ... '9':   //十进制
            {
                char* end = NULL;
                sym->value.uint = strtoull(next - 1, &end, base);
                sym->data_type = UINT;
                sym->token_type = Num;
                return end;
            }
        case '\'':  //字符
            sym->token_type = Num;
            sym->value.uint = token;
            break;
        case '"':  //字符串
            //TODO : 暂不支持转义字符
            {
                auto end = strchr(next, token);
                sym->value.str = strndup(next - 1, end - next + 1);
                sym->data_type = STR;
                sym->token_type = token;
                return end + 1;
            }
        case '/':   //TODO : 不支持多行注释
            if (*next == '/')
            {
                next = strchr(next, '\n');
                continue;
            }
            else
            {
                sym->token_type = Div;
                return next;
            }
        case '=':
            if(*next != '=')
            {
                sym->token_type = Assign;
                return next;
            }
            else
            {
                sym->token_type = Eq;
                return next + 1;
            }
        case '+':
            switch (*next)
            {
            case '+':
                sym->token_type = Inc;
                return next + 1;
                break;
            case '=':   //TODO +=
                break;
            default:
                sym->token_type = Add;
                return next;
            }
            break;
        case '~': 
        case ';': 
        case '{': 
        case '}': 
        case '(': 
        case ')': 
        case ']': 
        case ',': 
        case ':':
            sym->token_type = token;
            return next;
        default:
            break;
        }
    }
    return next;
}

//语法分析部分
int grammar(const char* src, symtab* symbols)
{
    symbol sym;
    int lineno = 0;

    src = find_token(src, &sym, lineno);
    if (src != NULL)
    {
        printf("%d - %s\n", sym.token_type, src);
    }

    while(sym.token_type != 0)
    {
        //src = find_token(src, &st, &sym, lineno);
        //if (src != NULL)
        //{
        //    printf("%s\n", src);
        //}
        src = find_token(src, &sym, lineno);
        printf("%d - %lu\n", sym.token_type, sym.value.uint);
    }
    printf("---------\n%s\n%d\n", src, sym.len);

    return 0;
}



static char t1[] = R"(
char else enum if int return sizeof while void main
)";

static char t2[] = R"(
#include <stdio.h>
int main()
{
    int a;
    a = 1 + 2;
    printf("%d\n", a);
    return 0;
}
)";


int main(int argc, char** argv)
{
    symtab st;
    symbol sym;
    int lineno = 0;

    const char* src = t1;
    while (*src != '\0')
    {
        src = find_token(src, &sym, lineno);
        if (sym.len > 0)
        {
                ////token.name = std::string(next - 1, end);
            st.add(sym);
            //printf("%s\n", src);
        }
    }

    //src = t2;
    //while (*src != '\0')
    //{
    //    src = find_token(src, &st, &sym, lineno);
    //    if (src != NULL)
    //    {
    //        printf("%s\n", src);
    //    }
    //}

    grammar(t2, &st);
    ////std::array<int, 4> dk = {'0', 'A', '_', 'a'};
    ////std::array<int, 4> dk = {'a', '_', 'A', '0'};
    //int dk[] = {'a', '_', 'A', '0'};

    //struct keyword
    //{
    //    int k;
    //    int l;
    //};
    //keyword ks[] = {{'a', 26}, {'_', 1}, {'A', 26}, {'0', 10}};

    ////keywords ks = {4, {'a', '_', 'A', '0'}, {26, 1, 26, 10}};

    //const char* t = "abc2q1 GJ-571";
    //while (*t != '\0')
    //{
    //    keyword* r = std::lower_bound(ks, ks + 4, *t, std::greater<int>());
    //    printf("%c - %p %p %c\n", *t, r, dk + 4, *r);
    //    t++;
    //}
    return 0;
}

