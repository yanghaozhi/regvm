#pragma once

#include <assert.h>

#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>

#include "common.h"

#include "lru.h"

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

    inst(const char* n, int i, int r, int e, const std::vector<int>& args);

    void recalc();

    void print(FILE* fp);
    void print_bin(FILE* fp);
    void print_txt(FILE* fp);

    int                 bytes;

    int                 id;
    int                 reg;
    int                 ex;
    const char*         name;
    uv                  val;
    std::string         str;

    std::vector<int>    args;
};


