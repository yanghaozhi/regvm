#pragma once

#include <string_view>
#include <unordered_map>

#include "common.h"

#define INST(c, r, e, ...)   insts.emplace_back(#c, CODE_##c, r, e, ##__VA_ARGS__);

enum EXTRA_CODE
{
    CODE_SETC    = 256,
    CODE_SETD,
};

struct inst
{
    inst(const char* n, int i, int r, int e);

    inst(const char* n, int i, int r, const std::string_view& v);

    inst(const char* n, int i, int r, int e, uv v);

    void print(FILE* fp);
    void print_bin(FILE* fp);
    void print_txt(FILE* fp);

    int                 id;
    int                 reg;
    int                 ex;
    const char*         name;
    uv                  val;
    std::string_view    str;
};

class sel_reg
{
public:
    sel_reg();
    int get(const char* name);
    inline int get(void)   {return used(0);}
    void clear(const char* name)    {names.erase(name);};
private:
    int used(int id);

    int8_t      regs[16];
    std::unordered_map<std::string_view, int>    names;
};

extern sel_reg         regs;


