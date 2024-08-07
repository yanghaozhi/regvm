#pragma once

#include "parser.h"

#include <labels.h>


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
    template <typename T> const char* optimize(const char* src, const std::string_view& name, T& reload, int op_id);
    virtual const char* go(const char* src, const token* toks, int count);
};

struct jumps : public parser::op
{
    jumps(parser* p) : parser::op(p)    {};
    virtual const char* jcmp(const char* src, labels<int>& js, int label);
};

struct if_else : public jumps
{
    if_else(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

//struct do_while : public jumps
//{
//    do_while(parser* p);
//    virtual const char* go(const char* src, const token* toks, int count);
//};
//
//struct while_loop : public jumps
//{
//    while_loop(parser* p);
//    virtual const char* go(const char* src, const token* toks, int count);
//};
//
//struct for_loop : public jumps
//{
//    for_loop(parser* p);
//    virtual const char* go(const char* src, const token* toks, int count);
//};
//
