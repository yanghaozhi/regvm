#pragma once

#include <assert.h>

#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>

#include "structs.h"


#define INST(c, r, ...)   insts.emplace_back(new instv<CODE_##c>(#c, r, ##__VA_ARGS__));

struct inst
{
    inst(int id, const char* name);
    virtual ~inst()                     {}

    int                 id;
    const char*         name;

    virtual bool scan(const char* str)      = 0;
    virtual int count(void) const           {return 1;};
    virtual void print(FILE* fp) const      = 0;
    virtual void print_bin(FILE* fp) const  = 0;
    virtual void print_asm(FILE* fp) const  = 0;
    virtual inst* copy() const              = 0;
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
    virtual bool scan(const char* str)
    {
        return sscanf(str, "%d %d %d", &a, &b, &c);
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
    virtual inst* copy() const
    {
        return new instv<N>(name);
    }
};


template <> struct instv<CODE_SET> : public inst, public instex
{
    int             reg;
    int             type;
    int             c;
    core::uvalue    ex;

    instv(const char* n) : inst(CODE_SET, n)   {}
    instv(const char* n, int r, const char* v);
    instv(const char* n, int r, uint64_t v);
    instv(const char* n, int r, int64_t v);
    instv(const char* n, int r, double v);

    virtual int count(void) const     {return 1 + datas.size();};
    virtual bool scan(const char* str);
    virtual void print(FILE* fp) const;
    virtual void print_bin(FILE* fp) const;
    virtual void print_asm(FILE* fp) const;
    virtual inst* copy() const          { return new instv<CODE_SET>(name); }
    bool change_str(const char* n);
};
template struct instv<CODE_SET>;


template <> struct instv<CODE_JUMP> : public inst
{
    int             offset;

    instv(const char* n) : inst(CODE_JUMP, n)   {}
    instv(const char* n, int o) : inst(CODE_JUMP, n)    {};

    virtual bool scan(const char* str);
    virtual int count(void) const       { return 1; };
    virtual inst* copy() const          { return new instv<CODE_JUMP>(name); }
    virtual void print(FILE* fp) const;
    virtual void print_bin(FILE* fp) const;
    virtual void print_asm(FILE* fp) const;
};
template struct instv<CODE_JUMP>;


