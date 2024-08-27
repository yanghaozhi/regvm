#include "statements.h"

#include <stdio.h>

#include <log.h>

#include "func.h"
#include "parser.h"

extern bool can_literally_optimize(const token& a, int& v);

template <typename T> var_crtp<T>::var_crtp(parser* p) : parser::op(p)
{
}

template <typename T> const char* var_crtp<T>::go(const char* src, const token* toks, int count)
{
    func::variable var;
    for (int i = 0; i < count - 1; i ++)
    {
        if (f->fill_var(toks[i], var) == false)
        {
            return NULL;
        }
    }
    return static_cast<T*>(this)->go2(src, toks + 2, count - 2, var);
}

decl_var_only::decl_var_only(parser* p) : var_crtp<decl_var_only>(p)
{
    p->add(this, Int, Id, ';', -1);
    p->add(this, Double, Id, ';', -1);
}

const char* decl_var_only::go2(const char* src, const token* toks, int count, func::variable& var)
{
    if ((var.attr & REG) == 0)
    {
        auto v2 = f->scopes.new_var(var.name, var.attr);
        if ((v2 == NULL) || (v2->reg.ptr == NULL))
        {
            LOGE("Can not new reg var : %s", VIEW(var.name));
            return NULL;
        }
        auto n = f->regs.tmp();
        INST(SET, (int)n, TYPE_ADDR, v2->id);
        INST(STORE, var.type, n, 3);
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

const char* decl_var_init::go2(const char* src, const token* toks, int count, func::variable& var)
{
    selector::reg v;
    src = f->expression(src, v);
    if (src == NULL) return NULL;

    auto r = f->scopes.bind_var(var.name, v, var.attr);
    if (r == NULL)
    {
        auto r = f->scopes.new_var(var.name, var.attr);
        if ((r == NULL) || (r->reg.ptr == NULL))
        {
            LOGE("Can not create var : %s", VIEW(var.name));
            return NULL;
        }
        INST(MOVE, r->reg, v, 0);
        v = r->reg;
    }

    if ((var.attr & REG) == 0)
    {
        auto n = f->regs.tmp();
        INST(SET, n, TYPE_ADDR, r->id);
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
    int ret;
    return f->call(src, toks[0].name, ret);
}

ret_func::ret_func(parser* p) : parser::op(p)
{
    p->add(this, Return, -1);
}

const char* ret_func::go(const char* src, const token* toks, int count)
{
    switch (f->rets.size())
    {
    case 0:
        break;
    case 1:
        {
            selector::reg v;
            src = f->expression(src, v);
            INST(CONV, f->infos.ret, v, f->rets[0].type);
        }
        break;
    default:
        break;
    }
    INST(RET);
    return src;
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
    auto reload = [this, name](int attr, uint64_t id)
        {
            auto n = f->regs.tmp();
            INST(SET, n, TYPE_ADDR, id);
            auto vv = f->regs.tmp();
            INST(LOAD, vv, n, 0);
            return vv;
        };
    int attr = 0;
    auto k = f->scopes.find_var(name, attr, reload);

    selector::reg v;
    src = f->expression(src, v);
    switch (toks[1].info.type)
    {
    case Assign:
        INST(MOVE, k->reg, v, 0);
        break;
#define CALC(i, op)                                     \
    case i:                                             \
        INST(op, k->reg, k->reg, v);                    \
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
        INST(STORE, k->reg, 0, 0);
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
    auto reload = [this, name](int attr, uint64_t id)
        {
            auto n = f->regs.tmp();
            INST(SET, n, TYPE_ADDR, id);
            auto vv = f->regs.tmp();
            INST(LOAD, vv, n, 0);
            return vv;
        };
    auto k = f->scopes.find_var(name, attr, reload);

    int v = 0;
    bool o = can_literally_optimize(toks[2], v);
    switch (toks[1].info.type)
    {
#define CALC(i, op, op2)                                        \
    case i:                                                     \
        if (o == false)                                         \
        {                                                       \
            auto r = f->regs.tmp();                             \
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
            INST(op, k->reg, k->reg, r);                        \
        }                                                       \
        else                                                    \
        {                                                       \
            INST(CALC, k->reg, v, op2);                         \
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
        INST(STORE, k->reg, 0, 0);
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

inline const char* cmp_jump_expr(const char* src, int label, func* f, labels<int>& j, int (*cmp)(int))
{
    selector::reg r;
    int end = 0;
    bool result = false;
    src = f->expression(src, r, &end, [&j, &result, label, cmp](func* f, int op, const token& a, const token* b, const selector::reg* r)
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
            auto& insts = f->insts;
            int c = -1;
            c = cmp(op);
            int ll = 0;
            selector::reg ret;
            if (can_literally_optimize(a, ll) == false)
            {
                ret = f->token_2_reg(a);
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
                    ret = f->token_2_reg(*b);
                    rr = ret;
                }
                else
                {
                    c |= 0x40;
                }
            }
            INST(JCMP, ll, rr, c, 0);
            j.jump(label, f->insts->back(), f->insts->size());
            if (ret.ptr == NULL)
            {
                ret.ptr = (decltype(ret.ptr))(uintptr_t)-1;
            }
            return ret;
        });

    auto& insts = f->insts;
    if (result == false)
    {
        INST(JCMP, r, 1, (cmp != cmp_op) | 0x40, 0);
        j.jump(label, f->insts->back(), f->insts->size());
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

    src = cmp_jump_expr(src, 0, f, jump, cmp_op_not);

    src = f->statements(src, NULL);

    token tok;
    auto o = f->parse->next_token(src, tok);
    if (tok.info.type == Else)
    {
        INST(JUMP, 0);
        jump.jump(1, insts->back(), insts->size());

        jump.label(0, insts->size());

        src = f->statements(o, NULL);

        jump.label(1, insts->size());
    }
    else
    {
        jump.label(0, insts->size());
    }
    return (jump.finish(*f->insts) == true) ? src : NULL;
}

do_while::do_while(parser* p) : parser::op(p)
{
    p->add(this, Do, '{', -1);
}

const char* do_while::go(const char* src, const token* toks, int count)
{
    labels<int> jump;

    jump.label(0, insts->size());

    src = f->statements(src - 1, jump, 2, 1);

    jump.label(1, insts->size());

    token tok;
    src = f->parse->next_token(src, tok);
    if (tok.info.type != While)
    {
        LOGE("do {...} MUST end with while !!!");
        return NULL;
    }

    src = cmp_jump_expr(src, 0, f, jump, cmp_op);

    jump.label(2, insts->size());

    return (jump.finish(*f->insts) == true) ? src : NULL;
}

while_loop::while_loop(parser* p) : parser::op(p)
{
    p->add(this, While, '(', -1);
}

const char* while_loop::go(const char* src, const token* toks, int count)
{
    labels<int> jump;

    jump.label(0, insts->size());

    src = cmp_jump_expr(src, 2, f, jump, cmp_op_not);

    src = f->statements(src, jump, 2, 0);

    INST(JUMP, 0);
    jump.jump(0, f->insts->back(), f->insts->size());

    jump.label(2, insts->size());

    return (jump.finish(*f->insts) == true) ? src : NULL;
}

for_loop::for_loop(parser* p) : parser::op(p)
{
    p->add(this, For, '(', -1);
}

const char* for_loop::go(const char* src, const token* toks, int count)
{
    labels<int> jump;

    src = f->statement(src);

    jump.label(0, insts->size());

    insts_t expr3;
    switch (*(src - 1))
    {
    case ';':
        {
            src = cmp_jump_expr(src, 5, f, jump, cmp_op_not);

            expr3.swap(*f->insts);
            src = f->statement(src);
            expr3.swap(*f->insts);

            token tok;
            src = f->parse->next_token(src, tok);
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

    src = f->statements(src, jump, 5, 0);

    for (auto& it : expr3)
    {
        insts->emplace_back(it);
    }

    INST(JUMP, 0);
    jump.jump(0, f->insts->back(), f->insts->size());

    jump.label(5, insts->size());

    return (jump.finish(*f->insts) == true) ? src : NULL;
}

decl_func::decl_func(parser* p) : var_crtp<decl_func>(p)
{
    p->add(this, Int, Id, '(', -1);
    p->add(this, Double, Id, '(', -1);
    p->add(this, Register, Int, Id, '(', -1);
    p->add(this, Register, Double, Id, '(', -1);
}

const char* decl_func::go2(const char* src, const token* toks, int count, func::variable& var)
{
    func* cur = NULL;
    auto it = p->funcs.find(var.name);
    if (it != p->funcs.end())
    {
        if (it->second.insts->size() > 0)
        {
            LOGW("function %d:%s is already exists !!!", it->second.id, VIEW(it->first));
            return NULL;
        }
        cur = &it->second;
        src = strchr(src, ')') + 1;
    }
    else
    {
        cur = &p->funcs.try_emplace(var.name, p, var.name).first->second;

        cur->rets.emplace_back(var);

        src = f->comma(src, [this, var, cur](const char* src, int* end)
            {
                func::variable arg;
                token tok;
                while ((src != NULL) && (*src != '\0'))
                {
                    src = p->next_token(src, tok);
                    switch (tok.info.type)
                    {
                    case ',':
                    case ';':
                    case ')':
                        *end = tok.info.type;
                        cur->args.emplace_back(arg);
                        return src;
                    default:
                        if (f->fill_var(tok, arg) == false)
                        {
                            src = NULL;
                            return src;
                        }
                        break;
                    }
                }
                return src;
            });
    }

    src = cur->go(src);
    return src;
}


