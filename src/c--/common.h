#pragma once

#include <code.h>

#include <log.h>
#include "selector.h"

//支持的标记类别(供词法分析器next解析成对应的标记)
enum TOKEN_T
{
    S       = 256,   //避免和ascii字符冲突
    Num     = 260, Int, Double, Str, Id, Register,
    If      = 270, Else, For, Do, While, Break, Continue,
    Assign  = 280, AddE, SubE, MulE, DivE, ModE, ShlE, ShrE,
    And     = 290, Or, Not, Eq, Ne, Lt, Gt, Le, Ge,
    Xor     = 300, Shl, Shr,
    Add     = 310, Sub, Mul, Div, Mod, Inc, Dec,
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
    selector::reg       reg;
};

const char* next_token(const char* src, int& lineno, token& tok);

#define VIEW(x) std::string(x).c_str()
