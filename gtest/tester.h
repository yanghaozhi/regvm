#pragma once

#include <gtest/gtest.h>

#include "structs.h"


#include <mem_run.h>
#include <debugger.h>


template <typename T> int check_size(void* ptr)
{
    T* p = (T*)ptr;
    return p->size();
}

#define CHECK_TYPE_SIGNED(val)      EXPECT_EQ(val, info->value.sint)
#define CHECK_TYPE_UNSIGNED(val)    EXPECT_EQ(val, info->value.uint)
#define CHECK_TYPE_DOUBLE(val)      EXPECT_DOUBLE_EQ(val, info->value.dbl)
#define CHECK_TYPE_STRING(val)      EXPECT_STREQ(val, info->value.str)
#define CHECK_TYPE_LIST(val)        EXPECT_EQ(val, check_size<core::uvalue::list_t>(info->value.ptr))
#define CHECK_TYPE_DICT(val)        EXPECT_EQ(val, check_size<core::uvalue::dict_t>(info->value.ptr))

#define CHECK_UV(K2, TYPE, VAL, REF, ATTR)                      \
    CHECK_##TYPE(VAL) << "at TRAP " << K2;                      \
    EXPECT_EQ(TYPE, info->type) << "at TRAP " << K2;            \
    EXPECT_EQ(ATTR, info->attr) << "at TRAP " << K2;            \
    EXPECT_EQ(REF, info->ref) << "at TRAP " << K2;

#define CHECK_FROM_Y    EXPECT_NE(nullptr, info->from)
#define CHECK_FROM_N    EXPECT_EQ(nullptr, info->from)

#define CHECK_REG(K1, K2, REG, FROM, ...)                       \
    if ((K1 == K2) && (info->id == REG))                        \
    {                                                           \
        match += 1;                                             \
        CHECK_UV(K2, __VA_ARGS__);                              \
        CHECK_FROM_##FROM << "at TRAP " << K2;                  \
    }

#define CHECK_VAR(K1, K2, NAME, CALL, SCOPE, REG, ...)          \
    if ((K1 == K2)                                              \
        && (strcmp(NAME, info->var_name) == 0)                  \
        && (CALL == info->call_id)                              \
        && (SCOPE == info->scope_id))                           \
    {                                                           \
        match += 1;                                             \
        CHECK_UV(K2, __VA_ARGS__);                              \
        EXPECT_EQ(REG, info->reg) << "at TRAP " << K2;          \
    }


class test_base : public vasm::debugger
{
public:
    test_base()     {};

    int64_t go(char* txt);
    int64_t go(char* txt, bool expect);

protected:
    int             key;
    int             offset;

    virtual void trap(regvm* vm, code_t code, int offset);

    virtual int check_reg(const regvm_reg_info* info);
    virtual int check_var(const regvm_var_info* info);

private:
    struct trap_arg
    {
        test_base*  self;
        int         matches;
    };
    static void check_reg(void* arg, const regvm_reg_info* info);
    static void check_var(void* arg, const regvm_var_info* info);
};

template <typename R, typename V> class tester : public test_base
{
public:
    tester(R r, V v) : test_base(), reg(r), var(v)   {}

    virtual int check_reg(const regvm_reg_info* info)
    {
        return reg(key, offset, info);
    }

    virtual int check_var(const regvm_var_info* info)
    {
        return var(key, offset, info);
    }

private:
    R       reg;
    V       var;
};

