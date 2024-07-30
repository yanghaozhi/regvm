#pragma once

#include <assert.h>

#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>

#include "structs.h"


#define INST(c, r, ...)   insts.emplace_back(instv<CODE_##c>{#c, r, ##__VA_ARGS__});

enum EXTRA_CODE
{
    CODE_SETS    = 256, //string
    CODE_SETD,          //double
    CODE_SETI,          //sint
};

struct inst
{
    typedef void (*print_op)(inst*, FILE*);

    inst(print_op f1, print_op f2, print_op f3);

    void recalc(void);

    void print(FILE* fp);
    void print_bin(FILE* fp);
    void print_asm(FILE* fp);

    int                 type    = -1;
    int                 count   = 1;

    int                 id;
    uint32_t            a;
    uint32_t            b;
    uint32_t            c;
    core::uvalue        ex;
    const char*         name;
    std::string         str;

    std::vector<int>    datas;

    print_op            ops[3];

    int set_datas(uint64_t v, int header);
};

template <int N> struct instv : public inst
{
    instv(const char* n, int _a, int _b, int _c) : inst(print, print_bin, print_asm)
    {
        id = N;
        name = n;
        a = _a;
        b = _b;
        c = _c;
    }
    static void print(inst* self, FILE* fp)
    {
        fprintf(fp, "%8s\t%02X\t%d\t%d\t%d\n", self->name, self->id, self->a, self->b, self->c);
    }
    static void print_bin(inst* self, FILE* fp)
    {
        code_t code;
        code.id = self->id;
        code.a = self->a;
        code.b = self->b;
        code.c = self->c;
        fwrite(&code, sizeof(code_t), 1, fp);
    }
    static void print_asm(inst* self, FILE* fp)
    {
        fprintf(fp, "%8s\t%d\t%d\t%d\n", self->name, self->a, self->b, self->c);
    }
};

template <> struct instv<CODE_SET> : public inst
{
    instv(const char* n, int r, const std::string_view& v);
    instv(const char* n, int r, uint64_t v);
    instv(const char* n, int r, int64_t v);
    instv(const char* n, int r, double v);

    static void print(inst* self, FILE* fp);
    static void print_bin(inst* self, FILE* fp);
    static void print_asm(inst* self, FILE* fp);
};
template struct instv<CODE_SET>;

template <> struct instv<CODE_JUMP> : public inst
{
    instv(const char* n, int where);

    static void print(inst* self, FILE* fp);
    static void print_bin(inst* self, FILE* fp);
    static void print_asm(inst* self, FILE* fp);
};
template struct instv<CODE_JUMP>;


