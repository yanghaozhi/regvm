/*
 * This module is rewrite from c4 project :https://github.com/rswier/c4
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <map>
#include <string>
#include <vector>
#include <string_view>
#include <unordered_set>
#include <unordered_map>

#include <code.h>


//支持的标记类别(供词法分析器next解析成对应的标记)
enum TOKEN_T { Num = 256,   //避免和ascii字符冲突
    Str, Fun, Sys, Glo, Loc, Id, Else, Enum, If, Int, Double, Return, Sizeof, While, Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak};


union uv
{
    int64_t     sint;
    uint64_t    uint;
    double      dbl;
};

struct code
{
    code(const char* n, int i, int r, int e) :
        id(i), reg(r), ex(e), name(n)
    {};
    code(const char* n, int i, int r, const std::string_view& v) :
        id(i), reg(r), ex(TYPE_STRING), name(n), str(v)
    {};
    code(const char* n, int i, int r, int e, uv v) :
        id(i), reg(r), ex(e), name(n), val(v)
    {
        switch (ex)
        {
        case TYPE_SIGNED:
            if (-32768 <= v.sint && v.sint <= 32767)
            {
                id = CODE_SETS;
                name = "SETS";
            }
            else if (-2147483648 <= v.sint && v.sint <= 2147483647)
            {
                id = CODE_SETI;
                name = "SETI";
            }
            else
            {
                id = CODE_SETL;
                name = "SETL";
            }
            break;
        case TYPE_UNSIGNED:
            if (v.uint <= 0xFFFF)
            {
                id = CODE_SETS;
                name = "SETS";
            }
            else if (v.uint <= 0xFFFFFFFF)
            {
                id = CODE_SETI;
                name = "SETI";
            }
            else
            {
                id = CODE_SETL;
                name = "SETL";
            }
            break;
        case TYPE_DOUBLE:
            id = CODE_SETL;
            name = "SETL";
            break;
        default:
            break;
        }
    };
    int                 id;
    int                 reg;
    int                 ex;
    const char*         name;
    uv                  val;
    std::string_view    str;
};

#define INST(c, r, e, ...)   codes.emplace_back(#c, CODE_##c, r, e, ##__VA_ARGS__);

struct token
{
    struct
    {
        uv              value;
        DATA_TYPE       data_type;
        int             token;
    }                   info;

    std::string_view    name;
};

std::unordered_map<std::string_view, int> keywords;   //  name : TOKEN_T

struct var
{
    DATA_TYPE           type;
    uv                  init;
    std::string_view    name;
};

struct func
{
    DATA_TYPE           ret;
    std::vector<var>    args;
};

std::unordered_map<std::string_view, func> funcs;

class sel_reg
{
public:
    sel_reg()
    {
        for (int i = 0; i < (int)sizeof(regs); i++)
        {
            regs[i] = i;
        }
    }
    inline int get(void)   {return used(0);}
    int get(const char* name)
    {
        auto it = names.find(name);
        if (it != names.end())
        {
            int8_t* p = (int8_t*)memchr(regs, it->second, sizeof(regs));
            return used(p - regs);
        }
        else
        {
            int v = used(0);
            names.emplace(name, v);
            return v;
        }
    }
    void clear(const char* name)    {names.erase(name);};
private:
    int used(int id)
    {
        int v = regs[id];
        if (id != sizeof(regs) - 1)
        {
            memmove(regs + id, regs + id + 1, sizeof(regs) - id - 1);
            regs[sizeof(regs) - 1] = v;
        }
        return v;
    }

    int8_t      regs[16];
    std::unordered_map<std::string_view, int>    names;
};
sel_reg         regs;

//struct keyword
//{
//    int k;
//    int l;
//};
//const keyword TOKEN_CHAR[] = {{'a', 26}, {'_', 1}, {'A', 26}, {'0', 10}};
//const keyword NUMBER_CHAR[] = {{'a', 26}, {'_', 1}, {'A', 26}, {'0', 10}};

const char* next_token(const char* src, int& lineno, token* tok);

//语法分析部分
const char* parse_func(const std::string_view& name, DATA_TYPE ret, const char* src, int& lineno)
{
    auto r = funcs.emplace(name, func{});
    if (r.second == false)
    {
        return NULL;
    }
    r.first->second.ret = ret;

    token tok;
    src = next_token(src, lineno, &tok);
    while (tok.info.token != ')')
    {
    }
    return src;
}

const char* expression(std::vector<code>& codes, const char* src, int reg, DATA_TYPE type, int& lineno)
{
    token tok;
    src = next_token(src, lineno, &tok);
    switch (tok.info.token)
    {
    case Num:
        INST(SETS, reg, type, tok.info.value);
        break;
    default:
        break;
    }
    return src;
}

const char* declaration(std::vector<code>& codes, const char* src, DATA_TYPE type, int& lineno)
{
    //int data_type = SINT;
    //switch (tok.info.token)
    //{
    //case Int:
    //    data_type = SINT;
    //    break;
    //default:
    //    break;
    //}
    token tok;
    //获取变量名
    src = next_token(src, lineno, &tok);
    if (tok.info.token != Id)
    {
        fprintf(stderr, "%d : bad variable declaration of %s !!!\n", lineno, std::string(tok.name).c_str());
        return NULL;
    }
    auto name = tok.name;

    src = next_token(src, lineno, &tok);
    switch (tok.info.token)
    {
    case '(':
        //TODO 函数
        return NULL;
    case ';':
        INST(CLEAR, regs.get(), type);
        break;
    case Assign:
        {
            int reg = regs.get();
            src = expression(codes, src, reg, type, lineno);
            int n = regs.get();
            codes.emplace_back("SETC",  CODE_SETL,  n, name);
            INST(STORE, reg, n);
        }
        break;
    default:
        return NULL;
    }
    return src;
}

const char* statement(std::vector<code>& codes, const char* src, token& tok, int& lineno)
{
    //return false;
    return NULL;
}


bool grammar(std::vector<code>& codes, const char* src)
{
    printf("%s\n", src);

    token tok;
    int lineno = 0;

    while ((src != NULL) && (*src != '\0'))
    {
        src = next_token(src, lineno, &tok);

        auto it = keywords.find(tok.name);
        if (it != keywords.end())
        {
            //处理类型
            switch (it->second)
            {
            case Int:
                src = declaration(codes, src, TYPE_SIGNED, lineno);
                break;
            case Double:
                src = declaration(codes, src, TYPE_DOUBLE, lineno);
                break;
                //case Char:
                //    src = next_token(src, &tok, lineno);
                //    break;
                //case Enum:  //TODO
                //    break;
            default:
                src = statement(codes, src, tok, lineno);
                break;
            }
        }
        else
        {
            //TODO : 正常赋值/函数调用语句
            //src = next_token(src, lineno, &tok);
            //continue;
        }
    }

    return (src != NULL) ? true : false;
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

const char* next_token(const char* src, int& lineno, token* tok)
{
    const char* next = src;
    memset(&tok->info, 0, sizeof(tok->info));
    tok->name = "";
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
                whole_token(next - 1, &end);
                tok->info.token = Id;
                tok->name = std::string_view(next - 1, end - next + 1);
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
                tok->info.value.uint = strtoull(next - 1, &end, base);
                tok->info.data_type = TYPE_UNSIGNED;
                tok->info.token = Num;
                return end;
            }
        case '\'':  //字符
            tok->info.token = Num;
            tok->info.value.uint = token;
            break;
        case '"':  //字符串
            //TODO : 暂不支持转义字符
            {
                auto end = strchr(next, token);
                tok->info.data_type = TYPE_STRING;
                tok->info.token = token;
                tok->name = std::string_view(next - 1, end - next + 1);
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
                tok->info.token = Div;
                return next;
            }
        case '=':
            if(*next != '=')
            {
                tok->info.token = Assign;
                return next;
            }
            else
            {
                tok->info.token = Eq;
                return next + 1;
            }
        case '+':
            switch (*next)
            {
            case '+':
                tok->info.token = Inc;
                return next + 1;
            case '=':   //TODO +=
                break;
            default:
                tok->info.token = Add;
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
            tok->info.token = token;
            return next;
        default:
            break;
        }
    }
    return next;
}

static char t1[] = R"(
int a;
double b = 12.5;
int c = a * b;
)";


[[maybe_unused]]		
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
    keywords.emplace("else", Else);
    keywords.emplace("if", If);
    keywords.emplace("int", Int);
    keywords.emplace("double", Double);
    keywords.emplace("return", Return);
    keywords.emplace("sizeof", Sizeof);
    keywords.emplace("while", While);

    std::vector<code> codes;
    auto r = grammar(codes, t1);
    printf("grammar : %d\n", r);
    if (r == true)
    {
        for (auto& it : codes)
        {
            printf("%s\t%d\t%d\n", it.name, it.reg, it.ex);
        }
    }

    //auto r = grammar(t2);
    //printf("grammar : %d\n", r);


    return 0;
}

