#include "statements.h"

#include <stdio.h>

template <typename T> const char* var_crtp<T>::go(parser* p, const char* src, const token* toks, int count)
{
    DATA_TYPE type = TYPE_NULL;
    switch (toks[0].info.type)
    {
    case Int:
        type = TYPE_SIGNED;
        break;
    case Double:
        type = TYPE_DOUBLE;
        break;
    default:
        break;
    }
    return static_cast<T*>(this)->go2(p, src, toks + 2, count - 2, type, toks[1].name);
}

decl_var_only::decl_var_only(parser* p)
{
    p->add(this, Int, Id, ';', -1);
    p->add(this, Double, Id, ';', -1);
}

const char* decl_var_only::go2(parser* p, const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name)
{
    auto& insts = p->insts;
    int v = regs.get();
    INST(CLEAR, v, type);
    int n = regs.get();
    insts.emplace_back("SETC",  CODE_SETL,  n, name);
    INST(STORE, v, n);
    return src;
}

decl_var_init::decl_var_init(parser* p)
{
    p->add(this, Int, Id, Assign, -1);
    p->add(this, Double, Id, Assign, -1);
}

const char* decl_var_init::go2(parser* p, const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name)
{
    int v = -1;
    src = p->expression(src, v);
    int n = regs.get();
    auto& insts = p->insts;
    insts.emplace_back("SETC", CODE_SETL, n, name);
    INST(STORE, v, n);
    return src;
}

call_func_no_ret::call_func_no_ret(parser* p)
{
    p->add(this, Id, '(', -1);
}

const char* call_func_no_ret::go(parser* p, const char* src, const token* toks, int count)
{
    int c = 16;
    int8_t rets[16];
    return p->call_func(src, toks[0], c, rets);
}

assign_var::assign_var(parser* p)
{
    p->add(this, Id, Assign, -1);
}

const char* assign_var::go(parser* p, const char* src, const token* toks, int count)
{
    int v = -1;
    src = p->expression(src, v);
    int n = regs.get();
    auto& insts = p->insts;
    insts.emplace_back("SETC", CODE_SETL, n, toks[0].name);
    INST(STORE, v, n);
    return src;
}

