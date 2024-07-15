#pragma once

#include <code.h>

#include "select.h"

//支持的标记类别(供词法分析器next解析成对应的标记)
enum TOKEN_T
{
    Num = 256,   //避免和ascii字符冲突
    Int, Double, Str,
    Id, If, Else, For, Do, While, Break, Continue,
    Assign, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge,
    Shl, Shr,
    Add, AddE, Sub, SubE, Mul, MulE, Div, DivE, Mod, ModE,
    Inc, Dec
};


union uv
{
    int64_t     sint;
    uint64_t    uint;
    double      dbl;
};

struct token
{
    struct
    {
        uv              value;
        DATA_TYPE       data_type;
        int             type;
        char            orig;
    }                   info;

    std::string_view    name;
    select::reg         reg;
};

const char* next_token(const char* src, int& lineno, token& tok);


