#include "statements.h"


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
    int v = regs.get();
    src = p->expression(src, v, type);
    int n = regs.get();
    auto& insts = p->insts;
    insts.emplace_back("SETC", CODE_SETL, n, name);
    INST(STORE, v, n);
    return src;
}

