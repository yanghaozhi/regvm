#pragma once

#include "parser.h"

#include <labels.h>


enum ATTR
{
    REG     = 0x01,
};

template <typename T> struct var_crtp : public parser::op
{
    var_crtp(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

struct decl_var_only : public var_crtp<decl_var_only>
{
    decl_var_only(parser* p);
    const char* go2(const char* src, const token* toks, int count, func::variable& var);
};

struct decl_var_init : public var_crtp<decl_var_init>
{
    decl_var_init(parser* p);
    const char* go2(const char* src, const token* toks, int count, func::variable& var);
};

struct decl_func : public var_crtp<decl_func>
{
    decl_func(parser* p);
    const char* go2(const char* src, const token* toks, int count, func::variable& var);
};

struct ret_func : public parser::op
{
    ret_func(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
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

struct assign_equal : public parser::op
{
    assign_equal(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

struct if_else : public parser::op
{
    if_else(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

struct do_while : public parser::op
{
    do_while(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

struct while_loop : public parser::op
{
    while_loop(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

struct for_loop : public parser::op
{
    for_loop(parser* p);
    virtual const char* go(const char* src, const token* toks, int count);
};

