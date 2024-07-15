#pragma once

#include "parser.h"


template <typename T> struct var_crtp : public parser::op
{
    var_crtp(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

struct decl_var_only : public var_crtp<decl_var_only>
{
    decl_var_only(parser* p);
    const char* go2(const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name);
};

struct decl_var_init : public var_crtp<decl_var_init>
{
    decl_var_init(parser* p);
    const char* go2(const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name);
};

struct call_func_no_ret : public parser::op
{
    call_func_no_ret(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

struct assign_var : public parser::op
{
    assign_var(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

struct jumps : public parser::op
{
    struct label
    {
        inst*   code;
        int     begin;
        int     end;
    };

    jumps(parser* p);
    int calc_bytes(int begin, int end);
    int set_addr(inst* code, int begin, int end);
};

struct if_else : public jumps
{
    if_else(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

