#include "statements.h"

#include <stdio.h>

#include <log.h>
#include "parser.h"

extern bool can_literally_optimize(const token& a, int& v);

template <typename T> var_crtp<T>::var_crtp(parser* p) : parser::op(p)
{
}

template <typename T> const char* var_crtp<T>::go(const char* src, const token* toks, int count)
{
    DATA_TYPE type = TYPE_NULL;
    int attr = 0;
    std::string_view name;
    for (int i = 0; i < count - 1; i ++)
    {
        switch (toks[i].info.type)
        {
        case Int:
            type = TYPE_SIGNED;
            break;
        case Double:
            type = TYPE_DOUBLE;
            break;
        case Register:
            attr |= REG;
            break;
        case Id:
            name = toks[i].name;
            break;
        default:
            break;
        }
    }
    return static_cast<T*>(this)->go2(src, toks + 2, count - 2, type, name, attr);
}

decl_var_only::decl_var_only(parser* p) : var_crtp<decl_var_only>(p)
{
    p->add(this, Int, Id, ';', -1);
    p->add(this, Double, Id, ';', -1);
}

const char* decl_var_only::go2(const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name, int attr)
{
    if ((attr & REG) == 0)
    {
        auto n = p->regs.tmp();
        INST(SET, n, name);
        INST(STORE, type, n, 3);
    }
    else
    {
        auto v2 = p->scopes.new_var(name, attr);
        if (v2.ptr == NULL)
        {
            LOGE("Can not new reg var : %s", VIEW(name));
            return NULL;
        }
    }
    return src;
}

decl_var_init::decl_var_init(parser* p) : var_crtp<decl_var_init>(p)
{
    p->add(this, Int, Id, Assign, -1);
    p->add(this, Double, Id, Assign, -1);
    p->add(this, Register, Int, Id, Assign, -1);
    p->add(this, Register, Double, Id, Assign, -1);
}

const char* decl_var_init::go2(const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name, int attr)
{
    selector::reg v;
    src = p->expression(src, v);
    if (src == NULL) return NULL;

    if (p->scopes.bind_var(name, v, attr) == false)
    {
        auto v2 = p->scopes.new_var(name, attr);
        if (v2.ptr == NULL)
        {
            LOGE("Can not create var : %s", VIEW(name));
            return NULL;
        }
        INST(MOVE, v2, v, 0);
        v = v2;
    }

    if ((attr & REG) == 0)
    {
        auto n = p->regs.tmp();
        INST(SET, n, name);
        INST(STORE, v, n, 2);
    }
    return src;
}

call_func_no_ret::call_func_no_ret(parser* p) : parser::op(p)
{
    p->add(this, Id, '(', -1);
}

const char* call_func_no_ret::go(const char* src, const token* toks, int count)
{
    std::vector<selector::reg> rets;
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
    p->add(this, Id, ShlE, -1);
    p->add(this, Id, ShrE, -1);
}

const char* assign_var::go(const char* src, const token* toks, int count)
{
    const std::string_view& name = toks[0].name;
    auto reload = [this, name](int attr)
        {
            auto n = p->regs.tmp();
            INST(SET, n, name);
            auto vv = p->regs.tmp();
            INST(LOAD, vv, n, 0);
            return vv;
        };
    int attr = 0;
    auto k = p->scopes.find_var(name, attr, reload);

    selector::reg v;
    src = p->expression(src, v);
    switch (toks[1].info.type)
    {
    case Assign:
        INST(MOVE, k, v, 0);
        break;
#define CALC(i, op)                                     \
    case i:                                             \
        INST(op, k, k, v);                              \
        break;
        CALC(AddE, ADD);
        CALC(SubE, SUB);
        CALC(MulE, MUL);
        CALC(DivE, DIV);
        CALC(ModE, MOD);
        CALC(ShlE, SHR);
        CALC(ShrE, SHL);
#undef CALC
    }

    if ((attr & REG) == 0)
    {
        INST(STORE, k, 0, 0);
    }
    return src;
}

assign_equal::assign_equal(parser* p) : parser::op(p)
{
    p->add(this, Id, AddE, Num, -1);
    p->add(this, Id, SubE, Num, -1);
    p->add(this, Id, MulE, Num, -1);
    p->add(this, Id, DivE, Num, -1);
    p->add(this, Id, ModE, Num, -1);
    p->add(this, Id, ShlE, Num, -1);
    p->add(this, Id, ShrE, Num, -1);
}

const char* assign_equal::go(const char* src, const token* toks, int count)
{
    const std::string_view& name = toks[0].name;
    int attr = 0;
    auto reload = [this, name](int attr)
        {
            auto n = p->regs.tmp();
            INST(SET, n, name);
            auto vv = p->regs.tmp();
            INST(LOAD, vv, n, 0);
            return vv;
        };
    auto k = p->scopes.find_var(name, attr, reload);

    int v = 0;
    bool o = can_literally_optimize(toks[2], v);
    switch (toks[1].info.type)
    {
#define CALC(i, op, op2)                                        \
    case i:                                                     \
        if (o == false)                                         \
        {                                                       \
            auto r = p->regs.tmp();                             \
            switch (toks[2].info.type)                          \
            {                                                   \
            case TYPE_SIGNED:                                   \
                INST(SET, r, toks[2].info.value.sint);          \
                break;                                          \
            case TYPE_UNSIGNED:                                 \
                INST(SET, r, toks[2].info.value.uint);          \
                break;                                          \
            case TYPE_DOUBLE:                                   \
                INST(SET, r, toks[2].info.value.dbl);           \
                break;                                          \
            default:                                            \
                return NULL;                                    \
            }                                                   \
            INST(op, k, k, r);                                  \
        }                                                       \
        else                                                    \
        {                                                       \
            INST(CALC, k, v, op2);                              \
        }                                                       \
        break;
        CALC(AddE, ADD, 0);
        CALC(SubE, SUB, 1);
        CALC(MulE, MUL, 2);
        CALC(DivE, DIV, 3);
        CALC(ModE, MOD, 4);
        CALC(ShlE, SHR, 5);
        CALC(ShrE, SHL, 6);
#undef CALC
    }
    if ((attr & REG) == 0)
    {
        INST(STORE, k, 0, 0);
    }
    return src;
}

extern int cmp_op(int op);
int cmp_op_not(int op)
{
    switch (op)
    {
#define CALC(k, op)         \
    case k:                 \
        return op;
        CALC(Eq, 1);
        CALC(Ne, 0);
        CALC(Gt, 5);
        CALC(Ge, 4);
        CALC(Lt, 3);
        CALC(Le, 2);
#undef CALC
    default:
        return -10000;
    }
}

inline const char* cmp_jump_expr(const char* src, int label, parser* p, labels<int>& j, int (*cmp)(int))
{
    selector::reg r;
    int end = 0;
    bool result = false;
    src = p->expression(src, r, &end, [&j, &result, label, cmp](parser* p, int op, const token& a, const token* b, const selector::reg* r)
        {
            switch (op)
            {
            case Eq:
            case Ne:
            case Gt:
            case Ge:
            case Lt:
            case Le:
                result = true;
                break;
            default:
                result = false;
                return selector::reg();
            }
            auto& insts = p->insts;
            int c = -1;
            c = cmp(op);
            int ll = 0;
            selector::reg ret;
            if (can_literally_optimize(a, ll) == false)
            {
                ret = p->token_2_reg(a);
                ll = ret;
            }
            else
            {
                c |= 0x80;
            }
            int rr = 0;
            if (r != NULL)
            {
                ret = *r;
                rr = *r;
            }
            else
            {
                if (can_literally_optimize(*b, rr) == false)
                {
                    ret = p->token_2_reg(*b);
                    rr = ret;
                }
                else
                {
                    c |= 0x40;
                }
            }
            INST(JCMP, ll, rr, c, 0);
            j.jump(label, p->insts.back(), p->insts.size());
            if (ret.ptr == NULL)
            {
                ret.ptr = (decltype(ret.ptr))(uintptr_t)-1;
            }
            return ret;
        });

    auto& insts = p->insts;
    if (result == false)
    {
        INST(JCMP, r, 1, (cmp != cmp_op) | 0x40, 0);
        j.jump(label, p->insts.back(), p->insts.size());
    }
    return src;
}

if_else::if_else(parser* p) : parser::op(p)
{
    p->add(this, If, '(', -1);
}

const char* if_else::go(const char* src, const token* toks, int count)
{
    labels<int> jump;

    src = cmp_jump_expr(src, 0, p, jump, cmp_op_not);

    src = p->statement(src);

    token tok;
    auto o = p->next_token(src, tok);
    if (tok.info.type == Else)
    {
        INST(JUMP, 0);
        jump.jump(1, insts.back(), insts.size());

        jump.label(0, insts.size());

        src = p->statement(o);

        jump.label(1, insts.size());
    }
    else
    {
        jump.label(0, insts.size());
    }
    return (jump.finish(p->insts) == true) ? src : NULL;
}

do_while::do_while(parser* p) : parser::op(p)
{
    p->add(this, Do, '{', -1);
}

const char* do_while::go(const char* src, const token* toks, int count)
{
    labels<int> jump;

    jump.label(0, insts.size());

    src = p->statements(src - 1, jump, 2, 1);

    jump.label(1, insts.size());

    token tok;
    src = p->next_token(src, tok);
    if (tok.info.type != While)
    {
        LOGE("do {...} MUST end with while !!!");
        return NULL;
    }

    src = cmp_jump_expr(src, 0, p, jump, cmp_op);

    jump.label(2, insts.size());

    return (jump.finish(p->insts) == true) ? src : NULL;
}

while_loop::while_loop(parser* p) : parser::op(p)
{
    p->add(this, While, '(', -1);
}

const char* while_loop::go(const char* src, const token* toks, int count)
{
    labels<int> jump;

    jump.label(0, insts.size());

    src = cmp_jump_expr(src, 2, p, jump, cmp_op_not);

    src = p->statements(src, jump, 2, 0);

    INST(JUMP, 0);
    jump.jump(0, p->insts.back(), p->insts.size());

    jump.label(2, insts.size());

    return (jump.finish(p->insts) == true) ? src : NULL;
}

for_loop::for_loop(parser* p) : parser::op(p)
{
    p->add(this, For, '(', -1);
}

const char* for_loop::go(const char* src, const token* toks, int count)
{
    labels<int> jump;

    src = p->statement(src);

    jump.label(0, insts.size());

    insts_t expr3;
    switch (*(src - 1))
    {
    case ';':
        {
            src = cmp_jump_expr(src, 5, p, jump, cmp_op_not);

            expr3.swap(p->insts);
            src = p->statement(src);
            expr3.swap(p->insts);

            token tok;
            src = p->next_token(src, tok);
            if (tok.info.type != ')')
            {
                LOGE("for loop condition must end with ) !!!");
                return NULL;
            }
        }
        break;
    case ':':
        break;
    default:
        LOGE("Unexpect token : %c !!!", *(src - 1));
        return NULL;
    }

    src = p->statements(src, jump, 5, 0);

    for (auto& it : expr3)
    {
        insts.emplace_back(it);
    }

    INST(JUMP, 0);
    jump.jump(0, p->insts.back(), p->insts.size());

    jump.label(5, insts.size());

    return (jump.finish(p->insts) == true) ? src : NULL;
}


