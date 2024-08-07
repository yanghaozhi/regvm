#include "statements.h"

#include <stdio.h>

#include <log.h>
#include "parser.h"


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
    auto n = p->regs.tmp();
    INST(SET, n, name);
    INST(STORE, type, n, 3);
    return src;
}

decl_var_init::decl_var_init(parser* p) : var_crtp<decl_var_init>(p)
{
    p->add(this, Int, Id, Assign, -1);
    p->add(this, Double, Id, Assign, -1);
}

const char* decl_var_init::go2(const char* src, const token* toks, int count, DATA_TYPE type, const std::string_view& name)
{
    selector::reg v;
    src = p->expression(src, v);
    if (src == NULL) return NULL;

    auto n = p->regs.tmp();
    INST(SET, n, name);
    INST(STORE, v, n, 2);
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
}

template <typename T> const char* assign_var::optimize(const char* src, const std::string_view& name, T& reload, int op_id)
{
    token toks[2];
    src = p->next_token(src, toks[0]);
    switch (toks[0].info.type)
    {
    case TYPE_SIGNED:
    case TYPE_UNSIGNED:
        if ((toks[0].info.type != Num) || (toks[0].info.value.sint > 127) || (toks[0].info.value.sint < -127))
        {
            return NULL;
        }
        break;
    default:
        return NULL;
    }
    src = p->next_token(src, toks[1]);
    switch (toks[1].info.type)
    {
    case ';':
    case ')':
        break;
    default:
        return NULL;
    }
    auto reg = p->scopes.find_var(name, reload);
    INST(CALC, reg, toks[0].info.value.sint, op_id);
    return src;
}

const char* assign_var::go(const char* src, const token* toks, int count)
{
    const std::string_view& name = toks[0].name;
    auto reload = [this, name]()
        {
            auto n = p->regs.tmp();
            INST(SET, n, name);
            auto vv = p->regs.tmp();
            INST(LOAD, vv, n, 0);
            return vv;
        };

    const char* o = NULL;
    switch (toks[1].info.type)
    {
#define OPT(k, op)                                  \
    case k:                                         \
        o = optimize(src, name, reload, op);        \
        break;
        OPT(AddE, 0);
        OPT(SubE, 1);
        OPT(MulE, 2);
        OPT(DivE, 3);
        OPT(ModE, 4);
        OPT(ShlE, 5);
        OPT(ShrE, 6);
#undef OPT
    default:
        break;
    }
    if (o != NULL)
    {
        return o;
    }

    selector::reg v;
    src = p->expression(src, v);
    switch (toks[1].info.type)
    {
    case Assign:
        {
            auto n = p->regs.tmp();
            INST(SET, n, toks[0].name);
            INST(STORE, v, n, 0);
        }
        break;
#define CALC(k, op)                                         \
    case k:                                                 \
        {                                                   \
            auto reg = p->scopes.find_var(name, reload);    \
            INST(op, reg, reg, v);                          \
            INST(STORE, reg, 0, 0);                         \
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

const char* jumps::jcmp(const char* src, labels<int>& js, int label)
{
    token toks[3];
    int end = 0;


    token v;
    src = p->next_token(src, v);
    if (v.info.type != '(') return NULL;

    src = p->expression(src, toks[0].reg, &end);
    if (src == NULL) return NULL;

    src = p->next_token(src, toks[1]);

    src = p->expression(src, toks[2].reg, &end);

    src = p->next_token(src, v);
    if (v.info.type != ')') return NULL;


    int op = 0;

#define REG_OR_NUM(v, idx)                              \
    int v = toks[idx].reg;                              \
    if (toks[idx].info.type == Num)                     \
    {                                                   \
        v = toks[0].info.value.sint;                    \
        if ((v <= 127) && (v >= -127))                  \
        {                                               \
            op &= 0x02;                                 \
        }                                               \
        else                                            \
        {                                               \
            auto n = p->regs.tmp();                     \
            INST(SET, n, (int64_t)v);                   \
            v = n;                                      \
        }                                               \
    }
    REG_OR_NUM(a, 0);
    REG_OR_NUM(b, 2);
#undef REG_OR_NUM

    switch (toks[1].info.type)
    {
#define CMP(k, v)                   \
    case k:                         \
        op += (v << 2);             \
        break;
        CMP(Eq, 0);
        CMP(Ne, 1);
        CMP(Gt, 2);
        CMP(Ge, 3);
        CMP(Lt, 4);
        CMP(Le, 5);
#undef CMP
    default:
        return NULL;
    }

    INST(JCMP, a, b, op);
    js.jump(label, insts.back(), insts.size());

    return src;
}

if_else::if_else(parser* p) : jumps(p)
{
    p->add(this, If, '(', -1);
}

const char* if_else::go(const char* src, const token* toks, int count)
{
    //selector::reg cmp;
    //src = p->expression(src, cmp);

    labels<int> jump;

    src = jcmp(src, jump, 0);
    if (src == NULL) return NULL;

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

//do_while::do_while(parser* p) : parser::op(p)
//{
//    p->add(this, Do, '{', -1);
//}
//
//const char* do_while::go(const char* src, const token* toks, int count)
//{
//    labels jump(insts);
//
//    jump.set_label(0);
//
//    src = p->statement(src - 1, [&jump](auto& tok)
//        {
//            switch (tok.info.type)
//            {
//            case Break:
//                SET_JUMP(jump, 2, JUMP, -1);
//                break;
//            case Continue:
//                SET_JUMP(jump, 1, JUMP, -1);
//                break;
//            default:
//                break;
//            }
//        });
//
//    jump.set_label(1);
//
//    token tok;
//    src = p->next_token(src, tok);
//    if (tok.info.type != While)
//    {
//        LOGE("do {...} MUST end with while !!!");
//        return NULL;
//    }
//
//    selector::reg cmp;
//    src = p->expression(src, cmp);
//
//    SET_JUMP(jump, 0, JNZ, cmp);
//
//    jump.set_label(2);
//
//    return (jump.finish() == true) ? src : NULL;
//}
//
//while_loop::while_loop(parser* p) : parser::op(p)
//{
//    p->add(this, While, '(', -1);
//}
//
//const char* while_loop::go(const char* src, const token* toks, int count)
//{
//    labels jump(insts);
//
//    jump.set_label(0);
//
//    selector::reg cmp;
//    src = p->expression(src, cmp);
//
//    SET_JUMP(jump, 2, JZ, cmp);
//
//    src = p->statement(src, [&jump](auto& tok)
//        {
//            switch (tok.info.type)
//            {
//            case Break:
//                SET_JUMP(jump, 2, JUMP, -1);
//                break;
//            case Continue:
//                SET_JUMP(jump, 0, JUMP, -1);
//                break;
//            default:
//                break;
//            }
//        });
//
//    SET_JUMP(jump, 0, JUMP, -1);
//
//    jump.set_label(2);
//
//    return (jump.finish() == true) ? src : NULL;
//}
//
//for_loop::for_loop(parser* p) : parser::op(p)
//{
//    p->add(this, For, '(', -1);
//}
//
//const char* for_loop::go(const char* src, const token* toks, int count)
//{
//    labels jump(insts);
//
//    src = p->statement(src);
//
//    jump.set_label(0);
//
//    std::deque<inst> expr3;
//    switch (*(src - 1))
//    {
//    case ';':
//        {
//            selector::reg cmp;
//            src = p->expression(src, cmp);
//            SET_JUMP(jump, 5, JZ, cmp);
//
//            p->regs.cleanup(true);
//
//            expr3.swap(insts);
//            src = p->statement(src);
//            expr3.swap(insts);
//        }
//        break;
//    case ':':
//        break;
//    default:
//        LOGE("Unexpect token : %c !!!", *(src - 1));
//        return NULL;
//    }
//
//    src = p->statement(src, [&jump](auto& tok)
//        {
//            switch (tok.info.type)
//            {
//            case Break:
//                SET_JUMP(jump, 5, JUMP, -1);
//                break;
//            case Continue:
//                SET_JUMP(jump, 0, JUMP, -1);
//                break;
//            default:
//                break;
//            }
//        });
//
//    for (auto& it : expr3)
//    {
//        insts.emplace_back(it);
//    }
//
//    SET_JUMP(jump, 0, JUMP, -1);
//
//    jump.set_label(5);
//
//    return (jump.finish() == true) ? src : NULL;
//}


