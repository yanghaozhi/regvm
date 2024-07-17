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
    switch (toks[1].info.type)
    {
    case Assign:
        {
            auto n = regs.tmp();
            INST(SETC, n, toks[0].name);
            INST(STORE, v, n);
        }
        break;
#define CALC(k, op)                                         \
    case k:                                                 \
        {                                                   \
            std::string_view name = toks[0].name;           \
            auto reg = regs.get(name, [this, name, v]()     \
                {                                           \
                    auto n = regs.tmp();                    \
                    INST(SETC, n, name);                    \
                    auto vv = regs.tmp();                   \
                    INST(LOAD, vv, n);                      \
                    return vv;                              \
                });                                         \
            INST(op, reg, v);                               \
            INST(STORE, reg, reg);                          \
        }                                                   \
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

class labels
{
public:
    labels(std::deque<inst>& i) : insts(i)  {}

    void set_jump(int label, const char* name, int code, int reg);
    void set_label(int label);
    bool finish();

private:
    struct jump
    {
        inst*   code;
        int     label;
        int     pos;
        int     to;
    };

    int64_t     label_min = 0xFFFFFFFFFF;
    int64_t     label_max = -1;

    std::deque<inst>&   insts;
    std::vector<jump>   js;
    std::map<int, int>  ls;

    int calc_bytes(int begin, int end)
    {
        int r = 0;
        auto b = insts.begin() + begin;
        auto e = insts.begin() + end;
        for (auto& p = b; p != e; ++p)
        {
            LOGT("%s - %d : %d : %d", p->name, p->bytes >> 1, p->reg, p->ex);
            r += p->bytes;
        }
        LOGT("--------------- %d -> %d = %d", begin, end, r >> 1);
        return r;
    }
};

#define SET_JUMP(j, l, c, r) j.set_jump(l, #c, CODE_##c, r);

void labels::set_jump(int label, const char* name, int code, int reg)
{
    uv pos;
    pos.sint = -1;
    auto addr = regs.tmp();
    INST(SETS, addr, TYPE_ADDR, pos);

    js.emplace_back(jump{&insts.back(), label, (int)insts.size() + 1, -1});

    insts.emplace_back(name, code, reg, (int)addr);
}

void labels::set_label(int label)
{
    int64_t cur = insts.size();
    if (cur < label_min)
    {
        label_min = cur;
    }
    if (cur > label_max)
    {
        label_max = cur;
    }
    ls.emplace(label, cur);
}

bool labels::finish()
{
    int64_t diff = (std::max(label_max, (int64_t)js.back().pos) - std::min(label_min, (int64_t)js.front().pos)) * 2;

    for (auto& it : js)
    {
        auto l = ls.find(it.label);
        if (l == ls.end())
        {
            LOGE("need to jump to label %d, but it is NOT exists !!!", it.label);
            return false;
        }

        it.code->val.sint = diff;
        it.code->ex = TYPE_SIGNED;
        it.code->recalc();
        it.to = l->second;
    }

    for (auto& it : js)
    {
        if (it.to > it.pos)
        {
            it.code->val.sint = (calc_bytes(it.pos, it.to) >> 1) + 1;
        }
        else
        {
            it.code->val.sint = -((calc_bytes(it.to, it.pos) - js.front().code->bytes) >> 1) - 1;
        }
    }

    return true;
}

if_else::if_else(parser* p) : parser::op(p)
{
    p->add(this, If, '(', -1);
}

const char* if_else::go(const char* src, const token* toks, int count)
{
    select::reg cmp;
    src = p->expression(src, cmp);

    labels jump(insts);

    SET_JUMP(jump, 0, JZ, cmp);

    src = p->statement(src);

    token tok;
    auto o = p->next_token(src, tok);
    if (tok.info.type == Else)
    {
        SET_JUMP(jump, 1, JUMP, -1);

        jump.set_label(0);

        src = p->statement(o);

        jump.set_label(1);
    }
    else
    {
        jump.set_label(0);
    }
    return (jump.finish() == true) ? src : NULL;
}

do_while::do_while(parser* p) : parser::op(p)
{
    p->add(this, Do, '{', -1);
}

const char* do_while::go(const char* src, const token* toks, int count)
{
    labels jump(insts);

    jump.set_label(0);

    src = p->statement(src - 1, [&jump](auto& tok)
        {
            switch (tok.info.type)
            {
            case Break:
                SET_JUMP(jump, 2, JUMP, -1);
                break;
            case Continue:
                SET_JUMP(jump, 1, JUMP, -1);
                break;
            default:
                break;
            }
        });

    jump.set_label(1);

    token tok;
    src = p->next_token(src, tok);
    if (tok.info.type != While)
    {
        LOGE("do {...} MUST end with while !!!");
        return NULL;
    }

    select::reg cmp;
    src = p->expression(src, cmp);

    SET_JUMP(jump, 0, JNZ, cmp);

    jump.set_label(2);

    return (jump.finish() == true) ? src : NULL;
}

while_loop::while_loop(parser* p) : parser::op(p)
{
    p->add(this, While, '(', -1);
}

const char* while_loop::go(const char* src, const token* toks, int count)
{
    labels jump(insts);

    jump.set_label(0);

    select::reg cmp;
    src = p->expression(src, cmp);

    SET_JUMP(jump, 2, JZ, cmp);

    src = p->statement(src - 1, [&jump](auto& tok)
        {
            switch (tok.info.type)
            {
            case Break:
                SET_JUMP(jump, 2, JUMP, -1);
                break;
            case Continue:
                SET_JUMP(jump, 0, JUMP, -1);
                break;
            default:
                break;
            }
        });

    SET_JUMP(jump, 0, JUMP, -1);

    jump.set_label(2);

    return (jump.finish() == true) ? src : NULL;
}

for_loop::for_loop(parser* p) : parser::op(p)
{
    p->add(this, For, '(', -1);
}

const char* for_loop::go(const char* src, const token* toks, int count)
{
    labels jump(insts);

    src = p->statement(src);

    jump.set_label(0);

    select::reg cmp;
    std::deque<inst> expr3;
    switch (*(src - 1))
    {
    case ';':
        src = p->expression(src, cmp);
        expr3.swap(insts);
        src = p->statement(src);
        expr3.swap(insts);
        break;
    case ':':
        break;
    default:
        LOGE("Unexpect token : %c !!!", *(src - 1));
        return NULL;
    }

    SET_JUMP(jump, 5, JZ, cmp);

    src = p->statement(src, [&jump](auto& tok)
        {
            switch (tok.info.type)
            {
            case Break:
                SET_JUMP(jump, 5, JUMP, -1);
                break;
            case Continue:
                SET_JUMP(jump, 0, JUMP, -1);
                break;
            default:
                break;
            }
        });

    for (auto& it : expr3)
    {
        insts.emplace_back(it);
    }

    SET_JUMP(jump, 0, JUMP, -1);

    jump.set_label(5);

    return (jump.finish() == true) ? src : NULL;
}


