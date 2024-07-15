#include "statements.h"

#include <stdio.h>

#include <log.h>

template <typename T> var_crtp<T>::var_crtp(parser* p) : parser::op(p)
{
}

template <typename T> const char* var_crtp<T>::go(const char* src, const token* toks, int count)
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
    return static_cast<T*>(this)->go2(src, toks + 2, count - 2, type, toks[1].name);
}

decl_var_only::decl_var_only(parser* p) : var_crtp<decl_var_only>(p)
{
    p->add(this, Int, Id, ';', -1);
    p->add(this, Double, Id, ';', -1);
}

const char* decl_var_only::go2(const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name)
{
    auto n = regs.tmp();
    INST(SETC, n, name);
    INST(NEW, n, type);
    return src;
}

decl_var_init::decl_var_init(parser* p) : var_crtp<decl_var_init>(p)
{
    p->add(this, Int, Id, Assign, -1);
    p->add(this, Double, Id, Assign, -1);
}

const char* decl_var_init::go2(const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name)
{
    select::reg v;
    src = p->expression(src, v);
    if (src == NULL) return NULL;

    auto n = regs.tmp();
    INST(SETC, n, name);
    INST(NEW, n, type);
    INST(STORE, v, n);
    regs.bind(name, v);
    return src;
}

call_func_no_ret::call_func_no_ret(parser* p) : parser::op(p)
{
    p->add(this, Id, '(', -1);
}

const char* call_func_no_ret::go(const char* src, const token* toks, int count)
{
    std::vector<select::reg> rets;
    return p->call_func(src, toks[0], rets);
}

assign_var::assign_var(parser* p) : parser::op(p)
{
    p->add(this, Id, Assign, -1);
    p->add(this, Id, AddE, -1);
    p->add(this, Id, SubE, -1);
    p->add(this, Id, MulE, -1);
    p->add(this, Id, DivE, -1);
    p->add(this, Id, ModE, -1);
}

const char* assign_var::go(const char* src, const token* toks, int count)
{
    select::reg v;
    src = p->expression(src, v);
    auto n = regs.tmp();
    INST(SETC, n, toks[0].name);
    switch (toks[0].info.type)
    {
    case Assign:
        INST(STORE, v, n);
        break;
#define CALC(k, op)                     \
    case k:                             \
        {                               \
            auto vv = regs.tmp();       \
            INST(LOAD, vv, n);          \
            INST(op, vv, v);            \
            INST(STORE, vv, n);         \
        }                               \
        break;
        CALC(AddE, ADD);
        CALC(SubE, SUB);
        CALC(MulE, MUL);
        CALC(DivE, DIV);
        CALC(ModE, MOD);
#undef CALC
    }
    return src;
}

jumps::jumps(parser* p) : parser::op(p)
{
}

int jumps::calc_bytes(int begin, int end)
{
    int r = 0;
    auto b = insts.begin() + begin;
    auto e = insts.begin() + end;
    for (auto& p = b; p != e; ++p)
    {
        LOGT("%s - %d : %d : %d", p->name, p->bytes >> 1, p->reg, p->ex);
        r += p->bytes;
    }
    LOGT("--------------- %d", r >> 1);
    return r;
}

int jumps::set_addr(label& l, bool backward)
{
    l.code->val.sint = (calc_bytes(l.begin, l.end) >> 1) + 1;
    if (backward == true)
    {
        l.code->val.sint = -(l.code->val.sint);
    }
    l.code->ex = TYPE_SIGNED;
    l.code->recalc();
    return l.code->val.sint;
}

if_else::if_else(parser* p) : jumps(p)
{
    p->add(this, If, '(', -1);
}

const char* if_else::go(const char* src, const token* toks, int count)
{
    LOGD("%s", src);
    select::reg cmp;
    src = p->expression(src, cmp);
    LOGD("%d, %s", (int)cmp, src);

    label labels[2];
    //int offsets[2];

    uv pos;
    pos.sint = -1;
    auto addr = regs.tmp();
    INST(SETS, addr, TYPE_ADDR, pos);
    labels[0].code = &insts.back();

    INST(JZ, cmp, addr);
    labels[0].begin = insts.size();

    src = p->statement(src);

    token tok;
    auto o = p->next_token(src, tok);
    if (tok.info.type == Else)
    {
        uv pos;
        pos.sint = -1;
        addr = regs.tmp();
        INST(SETS, addr, TYPE_ADDR, pos);
        labels[1].code = &insts.back();

        INST(JUMP, cmp, addr);
        labels[1].begin = insts.size();

        labels[0].end = insts.size();

        src = p->statement(o);

        labels[1].end = insts.size();

        set_addr(labels[1], false);
        set_addr(labels[0], false);
    }
    else
    {
        labels[0].end = insts.size();
        set_addr(labels[0], false);
    }
    return src;
}

do_while::do_while(parser* p) : jumps(p)
{
    p->add(this, Do, '{', -1);
}

const char* do_while::go(const char* src, const token* toks, int count)
{
    label l;
    l.begin = insts.size();

    src = p->statement(src - 1);

    token tok;
    src = p->next_token(src, tok);
    if (tok.info.type != While)
    {
        LOGE("do {...} MUST end with while !!!");
        return NULL;
    }

    select::reg cmp;
    src = p->expression(src, cmp);

    l.end = insts.size();

    uv pos;
    pos.sint = -1;
    auto addr = regs.tmp();
    INST(SETS, addr, TYPE_ADDR, pos);
    l.code = &insts.back();

    INST(JUMP, cmp, addr);
    set_addr(l, true);

    return src;
}
