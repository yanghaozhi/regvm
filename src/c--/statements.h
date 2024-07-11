#pragma once

#include "parser.h"


template <typename T> struct var_crtp : public parser::op
{
    virtual const char* go(parser* p, const char* src, const token* toks, int count);
};

struct decl_var_only : public var_crtp<decl_var_only>
{
    decl_var_only(parser* p);
    const char* go2(parser* p, const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name);
};

struct decl_var_init : public var_crtp<decl_var_init>
{
    decl_var_init(parser* p);
    const char* go2(parser* p, const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name);
};

struct call_func_no_ret : public parser::op
{
    call_func_no_ret(parser* p);
    virtual const char* go(parser* p, const char* src, const token* toks, int count);
};

