#include "func.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <deque>
#include <string>
#include <typeinfo>

#include <log.h>
#include <code.h>

#include "statements.h"




func::func(parser* p, const std::string_view& n) : id(p->func_id++), name(n), parse(p), insts(&instss), regs(p->regs), scopes(insts, regs)
{
}


func::~func()
{
    for (auto& it : instss)
    {
        delete it;
    }
}

const char* func::go(const char* src)
{
    infos.arg = infos.ret + rets.size();
    LOGT("func : %d - ret : %d, arg : %d", id, infos.ret, infos.arg);

    token tok;
    const char* p = parse->next_token(src, tok);
    switch (tok.info.type)
    {
    case ';':
        return p;
    case '{':
        scopes.enter();
        if (args.size() > 0)
        {
            int i = infos.arg;
            for (auto& it : args)
            {
                scopes.bind_arg(it.name, i++, it.attr);
            }
        }
        src = statements(src, NULL);
        scopes.leave();
        break;
    default:
        scopes.enter();
        while ((src != NULL) && (*src != '\0'))
        {
            src = statement(src);
        }
        scopes.leave();
        break;
    }

    return src;
}

void func::print(inst_print_t op, FILE* fp) const
{
    for (auto& it : instss)
    {
        (it->*op)(fp);
    }
}

const char* func::statements(const char* src, std::function<void (const token&)> cb)
{
    token tok;
    const char* p = parse->next_token(src, tok);
    switch (tok.info.type)
    {
    case '{':
        scopes.enter();
        //INST(BLOCK, 0, 0, 0);
        while ((p != NULL) && (tok.info.type != '}'))
        {
            p = statement(p, cb, tok);
        }
        //INST(BLOCK, 0, 1, 0);
        scopes.leave();
        return p;
    case 0:
    case ';':
        return p;
    default:
        return statement(src, cb, tok);
    }
}

const char* func::statement(const char* src, std::function<void (const token&)> cb, token& tok)
{
    const char* p = src;
    src = parse->next_token(src, tok);
    switch (tok.info.type)
    {
    case 0:
    case ';':
        return src;
    case '{':
        return statements(src, cb);
    case '}':
        return src;
    case Break:
    case Continue:
        if (cb)
        {
            cb(tok);
            return src;
        }
        else
        {
            LOGE("Do NOT allow use break/continue outside loops !!!");
            return NULL;
        }
    default:
        src = p;
        break;
    }

    p = parse->find_statement(src, this);
    if (p == NULL)
    {
        COMPILE_ERROR(parse, "Can NOT find any statement to deal with this line");
        return NULL;
    }
    return p;
}

const char* func::statement(const char* src)
{
    token tok;
    return statement(src, NULL, tok);
}

bool func::fill_var(const token& tok, variable& var)
{
    switch (tok.info.type)
    {
    case Int:
        var.type = TYPE_SIGNED;
        break;
    case Double:
        var.type = TYPE_DOUBLE;
        break;
    case Register:
        var.attr |= REG;
        break;
    case Id:
        var.name = tok.name;
        break;
    default:
        return false;
    }
    return true;
}

inline int calc_op(int op)
{
    switch (op)
    {
#define CALC(k, op)         \
    case k:                 \
        return op;
        CALC(Add, 0);
        CALC(Sub, 1);
        CALC(Mul, 2);
        CALC(Div, 3);
        CALC(Mod, 4);
        CALC(Shl, 5);
        CALC(Shr, 6);
#undef CALC
    default:
        return -10000;
    }
}

int cmp_op(int op)
{
    switch (op)
    {
#define CALC(k, op)         \
    case k:                 \
        return op;
        CALC(Eq, 0);
        CALC(Ne, 1);
        CALC(Gt, 2);
        CALC(Ge, 3);
        CALC(Lt, 4);
        CALC(Le, 5);
#undef CALC
    default:
        return -10000;
    }
}

inline int operator_level(int op)
{
    //ref : https://en.cppreference.com/w/cpp/language/operator_precedence
    switch (op)
    {
    case Add:
    case Sub:
        return 6;
    case Mul:
    case Div:
    case Mod:
        return 5;
    case Shl:
    case Shr:
        return 7;
    case Gt:
    case Ge:
    case Lt:
    case Le:
        return 9;
    case Eq:
        return 10;
    //case Xor:
    //    return 12;
    //case Or:
    //    return 13;
    default:
        return -1;
    }
}

selector::reg func::token_2_reg(const token& tok)
{
    if (regs.active(tok.reg) == true)
    {
        return tok.reg;
    }

    switch (tok.info.type)
    {
    case Num:
        {
            auto reg = regs.tmp();
            switch (tok.info.data_type)
            {
            case TYPE_SIGNED:
                INST(SET, reg, tok.info.value.sint);
                break;
            case TYPE_UNSIGNED:
                INST(SET, reg, tok.info.value.uint);
                break;
            case TYPE_DOUBLE:
                INST(SET, reg, tok.info.value.dbl);
                break;
            default:
                return selector::reg();
            }
            return reg;
        }
        break;
    case Id:
        {
            std::string_view name = tok.name;
            int attr = 0;
            auto v = scopes.find_var(tok.name, attr, [this, name](int attr, uint64_t id)
                {
                    auto n = regs.tmp();
                    INST(SET, n, TYPE_ADDR, id);
                    auto v = scopes.new_var(name, 0);
                    INST(LOAD, v->reg, n, 0);
                    return v->reg;
                });
            if ((v == NULL) || (v->reg.ptr == NULL))
            {
                COMPILE_ERROR(parse, "Can NOT find var : %s at func %d", VIEW(tok.name), id);
                return selector::reg();
            }
            return v->reg;
        }
        break;
    default:
        LOGE("%d : invalid expression %d - %s !!!", parse->lineno, tok.info.type, std::string(tok.name).c_str());
        break;
    }
    return selector::reg();
}

bool can_literally_optimize(const token& a, int& v)
{
    if (a.info.type != Num)
    {
        return false;
    }

    switch (a.info.data_type)
    {
    case TYPE_SIGNED:
        v = a.info.value.sint;
        if ((v < -127) || (v > 127)) return false;
        break;
    case TYPE_UNSIGNED:
        v = a.info.value.uint;
        if (v > 127) return false;
        break;
    default:
        return false;
    }
    return true;
}

inline bool literally_calc(func* p, int op, const token& a, const selector::reg& b)
{
    switch (b.ptr->status)
    {
    case selector::BINDED:
    case selector::LOCKED:
        return false;
    default:
        break;
    }

    int v = 0;
    if (can_literally_optimize(a, v) == false)
    {
        return false;
    }

    auto& insts = p->insts;
    switch (op)
    {
    case Add:
    case Mul:
        INST(CALC, b, v, calc_op(op));
        return true;
    default:
        return false;
    }
}

inline bool literally_cmp(func* p, int op, const token& a, const selector::reg& b)
{
    return false;
}

inline selector::reg calc_a_b(func* p, int op, const token& a, const selector::reg& b)
{
    auto& insts = p->insts;

    if ((300 <= op) && (op < 320))
    {
        if (literally_calc(p, op, a, b) == true)
        {
            return b;
        }
    }

    selector::reg r = p->regs.tmp();

    auto c = p->token_2_reg(a);
    switch (op)
    {
    case Add:
        INST(ADD, r, c, b);
        break;
    case Sub:
        INST(SUB, r, c, b);
        break;
    case Mul:
        INST(MUL, r, c, b);
        break;
    case Div:
        INST(DIV, r, c, b);
        break;
    case Mod:
        INST(MOD, r, c, b);
        break;
    case Eq:
    case Ne:
    case Gt:
    case Ge:
    case Lt:
    case Le:
        INST(SUB, r, c, b);
        INST(CMP, r, r, cmp_op(op));
        break;
    default:
        COMPILE_ERROR(p->parse, "UNKNOWN operator of expression %d - %c !!!", op, (char)op);
        return selector::reg();
    }
    return r;
}

inline selector::reg calc_a_b(func* p, int op, const token& a, const token& b)
{
    int v = 0;
    if (can_literally_optimize(b, v) == true)
    {
        auto& insts = p->insts;
        auto v1 = p->token_2_reg(a);
        switch (op)
        {
        case Add:
        case Sub:
        case Mul:
        case Div:
        case Mod:
            switch (v1.ptr->status)
            {
            case selector::BINDED:
            case selector::LOCKED:
                break;
            default:
                INST(CALC, v1, v, calc_op(op));
                return v1;
            }
            break;
        default:
            break;
        };
    }
    auto v2 = p->token_2_reg(b);
    if (can_literally_optimize(a, v) == true)
    {
        if (literally_calc(p, op, a, v2) == true)
        {
            return v2;
        }
    }
    return calc_a_b(p, op, a, v2);
}

template <typename T, typename O> selector::reg pop_and_calc(func* p, T& toks, O& ops, const func::calc_t& calc)
{
    if (toks.size() == 0)
    {
        LOGE("toks is empty");
        return selector::reg();
    }
    if (ops.size() == 0)
    {
        auto r = p->token_2_reg(toks.back());
        toks.pop_back();
        return r;
    }
    if (ops.size() != toks.size() - 1)
    {
        LOGE("size of ops(%zd) or size of toks(%zd) invalid !!!", ops.size(), toks.size());
        return selector::reg();
    }

    const auto& v2 = toks.back();
    selector::reg b;
    int level = -1;
    if (v2.info.type == Num)
    {
        int op = ops.back();
        const auto& v1 = toks[toks.size() - 2];
        if ((calc != NULL) && (ops.size() == 1))
        {
            auto r = calc(p, op, v1, &v2, NULL);
            if (r.ptr != NULL)
            {
                toks.clear();
                ops.clear();
                return r;
            }
        }
        b = calc_a_b(p, op, v1, v2);
        level = operator_level(op);
        toks.pop_back();
        ops.pop_back();
    }
    else
    {
        b = p->token_2_reg(v2);
    }
    toks.pop_back();

    while ((ops.size() > 0) && (level <= operator_level(ops.back())))
    {
        int op = ops.back();
        auto& a = toks.back();

        if ((calc != NULL) && (ops.size() == 1))
        {
            auto r = calc(p, op, a, NULL, &b);
            if (r.ptr != NULL)
            {
                toks.clear();
                ops.clear();
                return r;
            }
        }
        b = calc_a_b(p, op, a, b);

        toks.pop_back();
        ops.pop_back();
    }
    return b;
}

const char* func::call(const char* src, func& sub, int retc, std::vector<selector::reg>& rets)
{
    if ((size_t)retc > sub.rets.size())
    {
        COMPILE_ERROR(parse, "Call function %d ERROR, want %d rets, but only %zd give", sub.id, retc, sub.rets.size());
        return NULL;
    }

    int sub_info = 128 + rets.size() + args.size() + 1;

    int i = 0;
    const int begin = sub_info + sub.rets.size() + 1;
    src = comma(src, [this, &sub, &i, begin](const char* src, int* end)
        {
            selector::reg reg; 
            src = expression(src, reg, end, NULL);
            if (reg.ptr == NULL)
            {
                LOGE("invalid expression result : %d:%p : %s !!!", reg.ver, reg.ptr, src);
            }
            INST(MOVE, begin + i, (int)reg, sub.args[i].type);
            i += 1;
            return src;
        });
    if ((size_t)i != sub.args.size())
    {
        COMPILE_ERROR(parse, "Call function %d ERROR, need %zd args, but %d give", sub.id, sub.args.size(), i);
        return NULL;
    }

    int64_t info = sub.id;
    info <<= 32;
    info += (sub.rets.size() << 8) + sub.args.size();
    INST(SET, sub_info, TYPE_SIGNED, info);

    INST(CALL, sub_info, sub.id);

    for (int j = 0; j < retc; j++)
    {
        auto n = regs.tmp();
        INST(MOVE, n, sub_info + j + 1, sub.rets[j].type);
    }

    return src;
}

const char* func::expression(const char* src, selector::reg& reg)
{
    int end = 0;
    return expression(src, reg, &end, NULL);
}

const char* func::expression(const char* src, selector::reg& reg, int* end, const calc_t& calc)
{
    std::deque<token>   toks;
    std::deque<int>     ops;

    while ((src != NULL) && (*src != '\0'))
    {
        if (toks.size() == ops.size())
        {
            auto& v = toks.emplace_back();
            src = parse->next_token(src, v);
            switch (v.info.type)
            {
            case '(':
                src = expression(src, v.reg, end, calc);
                break;
            case Num:
            case Id:
                break;
            case ';':
                toks.pop_back();
                reg = pop_and_calc(this, toks, ops, calc);
                if (end != NULL)
                {
                    *end = ';';
                }
                return src;
            default:
                LOGE("%d : invalid token of expression %d - %c - %s !!!", parse->lineno, v.info.type, v.info.orig, std::string(v.name).c_str());
                return NULL;
            }
        }

        token op;
        src = parse->next_token(src, op);
        switch (op.info.type)
        {
        case ':':
        case ';':
        case ',':
        case ')':
            reg = pop_and_calc(this, toks, ops, calc);
            if (end != NULL)
            {
                *end = op.info.type;
            }
            return src;
        //case '(':   //函数调用
        //    {
        //        std::vector<selector::reg> rets;
        //        src = call_func(src, toks.back(), rets);
        //        if (rets.size() >= 1)
        //        {
        //            toks.back().reg = rets[0];
        //        }
        //    }
        //    break;
        default:
            {
                if (ops.size() > 0)
                {
                    int cur = operator_level(op.info.type);
                    int prev = operator_level(ops.back());
                    if (cur > prev)
                    {
                        auto rr = pop_and_calc(this, toks, ops, calc);
                        auto& vv = toks.emplace_back();
                        vv.reg = rr;
                    }
                }
                ops.emplace_back(op.info.type);
            }
            break;
        }
    }
    if (end != NULL)
    {
        *end = 0;
    }
    return src;
}



