#pragma once

#include <assert.h>

#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>

#include "structs.h"


struct inst;
typedef std::deque<inst*>               insts_t;

struct inst
{
    inst(int id, const char* name);
    virtual ~inst()                     {}

    int                 id;
    const char*         name;

    virtual bool scan(const char* src)      = 0;
    virtual int count(void) const           {return 1;};
    virtual void print(FILE* fp) const      = 0;
    virtual void print_bin(FILE* fp) const  = 0;
    virtual void print_asm(FILE* fp) const  = 0;
};

struct instex
{
    std::vector<int>    datas;
    int set_datas(uint64_t v, int header);
};

template <int N> struct instv : public inst
{
    uint32_t            a;
    uint32_t            b;
    uint32_t            c;

    instv(const char* n) : inst(N, n)   {}
    instv(const char* n, int _a, int _b, int _c) : inst(N, n), a(_a), b(_b), c(_c)  {}
    virtual bool scan(const char* src)
    {
        return sscanf(src, "%d %d %d", &a, &b, &c) == 3;
    }
    virtual void print(FILE* fp) const
    {
        fprintf(fp, "%-8s %02X\t%d\t%d\t%d\n", name, id, a, b, c);
    }
    virtual void print_bin(FILE* fp) const
    {
        code_t code;
        code.id = id;
        code.a = a;
        code.b = b;
        code.c = c;
        fwrite(&code, sizeof(code_t), 1, fp);
    }
    virtual void print_asm(FILE* fp) const
    {
        fprintf(fp, "%-8s %d\t%d\t%d\n", name, a, b, c);
    }
};

template <int N> inst* create_inst(const char* name)
{
    return new instv<N>(name);
}

template <> struct instv<CODE_SET> : public inst, public instex
{
    int             reg;
    int             type;
    int             c;
    core::uvalue    ex;
    std::string     str;

    instv(const char* n) : inst(CODE_SET, n)   {}
    instv(const char* n, int r, const std::string_view& v);
    instv(const char* n, int r, const char* v);
    instv(const char* n, int r, uint64_t v);
    instv(const char* n, int r, int t, uint64_t v);
    instv(const char* n, int r, int64_t v);
    instv(const char* n, int r, double v);

    virtual int count(void) const     {return 1 + datas.size();};
    virtual bool scan(const char* src);
    virtual void print(FILE* fp) const;
    virtual void print_bin(FILE* fp) const;
    virtual void print_asm(FILE* fp) const;
    bool change_str(const char* n);
};
template struct instv<CODE_SET>;


template <> struct instv<CODE_JUMP> : public inst
{
    int             offset;

    instv(const char* n) : inst(CODE_JUMP, n)   {}
    instv(const char* n, int o) : inst(CODE_JUMP, n)    {};

    instv(int i, const char* n) : inst(i, n)   {}
    instv(int i, const char* n, int o) : inst(i, n)    {};

    virtual bool scan(const char* src);
    virtual int count(void) const       { return 1; };
    virtual void print(FILE* fp) const;
    virtual void print_bin(FILE* fp) const;
    virtual void print_asm(FILE* fp) const;
};
template struct instv<CODE_JUMP>;


struct instj : public instv<CODE_JUMP>
{
    int     a;
    int     b;
    int     c;

    instj(int i, const char* n) : instv<CODE_JUMP>(i, n)    {};
    instj(int i, const char* n, int _a, int _b, int _c, int o) :
        instv<CODE_JUMP>(i, n, o), a(_a), b(_b), c(_c)    {};

    virtual bool scan(const char* src);
    virtual int count(void) const       { return 2; };
    virtual void print(FILE* fp) const;
    virtual void print_bin(FILE* fp) const;
    virtual void print_asm(FILE* fp) const;
};

#define JUMP_CMP(x)                                     \
template <> struct instv<x> : public instj              \
{                                                       \
    instv(const char* n) : instj(x, n)  {}              \
    instv(const char* n, int a, int b, int c, int o)    \
        : instj(x, n, a, b, c, o)  {}                   \
};
JUMP_CMP(CODE_JEQ);
JUMP_CMP(CODE_JNE);
JUMP_CMP(CODE_JGT);
JUMP_CMP(CODE_JGE);
JUMP_CMP(CODE_JLT);
JUMP_CMP(CODE_JLE);
#undef JUMP_CMP

template <> struct instv<CODE_JCMP> : public instv<CODE_JUMP>
{
    int     a;
    int     b;
    int     c;

    instv(const char* n) : instv<CODE_JUMP>(CODE_JCMP, n)    {};
    instv(const char* n, int _a, int _b, int _c, int o) :
        instv<CODE_JUMP>(CODE_JCMP, n, o), a(_a), b(_b), c(_c)    {};

    virtual bool scan(const char* src);
    virtual int count(void) const       { return 2; };
    virtual void print(FILE* fp) const;
    virtual void print_bin(FILE* fp) const;
    virtual void print_asm(FILE* fp) const;
};
template struct instv<CODE_JCMP>;

template <> struct instv<CODE_ECHO> : public inst
{
    std::vector<int>    args;

    instv(const char* n) : inst(CODE_ECHO, n)   {}
    instv(const char* n, const std::vector<int>& a) : inst(CODE_ECHO, n), args(a)   {}

    virtual int count(void) const;
    virtual bool scan(const char* src);
    virtual void print(FILE* fp) const;
    virtual void print_bin(FILE* fp) const;
    virtual void print_asm(FILE* fp) const;
    bool change_str(const char* n);
};
template struct instv<CODE_ECHO>;
