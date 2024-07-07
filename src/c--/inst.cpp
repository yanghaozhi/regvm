#include "inst.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <string>


sel_reg         regs;

enum INTERNAL_CODE
{
    SETC    = 256,
    SETD,
};

inst::inst(const char* n, int i, int r, int e) :
    id(i), reg(r), ex(e), name(n)
{
}

inst::inst(const char* n, int i, int r, const std::string_view& v) :
    id(SETC), reg(r), ex(TYPE_STRING), name("SETC"), str(v)
{
}

inst::inst(const char* n, int i, int r, int e, uv v) :
    id(i), reg(r), ex(e), name(n), val(v)
{
    switch (ex)
    {
    case TYPE_SIGNED:
        if (-32768 <= v.sint && v.sint <= 32767)
        {
            id = CODE_SETS;
            name = "SETS";
        }
        else if (-2147483648 <= v.sint && v.sint <= 2147483647)
        {
            id = CODE_SETI;
            name = "SETI";
        }
        else
        {
            id = CODE_SETL;
            name = "SETL";
        }
        break;
    case TYPE_UNSIGNED:
        if (v.uint <= 0xFFFF)
        {
            id = CODE_SETS;
            name = "SETS";
        }
        else if (v.uint <= 0xFFFFFFFF)
        {
            id = CODE_SETI;
            name = "SETI";
        }
        else
        {
            id = CODE_SETL;
            name = "SETL";
        }
        break;
    case TYPE_DOUBLE:
        id = CODE_SETL;
        name = "SETL";
        break;
    default:
        break;
    }
}

void inst::print(FILE* fp)
{
    fprintf(fp, "%s\t%02X\t%d\t%d\t%ld\t%s\n", name, id, reg, ex, val.sint, std::string(str).c_str());
}

void inst::print_bin(FILE* fp)
{
}

void inst::print_txt(FILE* fp)
{
    fprintf(fp, "%s\t%d\t%d", name, reg, ex);
    switch (id)
    {
    case SETC:
        fprintf(fp, "\t%s\n", std::string(str).c_str());
        break;
    case CODE_SETS:
    case CODE_SETI:
    case CODE_SETL:
        switch (ex)
        {
        case TYPE_SIGNED:
            fprintf(fp, "\t%lld\n", (long long)val.sint);
            break;
        case TYPE_UNSIGNED:
            fprintf(fp, "\t%llu\n", (long long)val.uint);
            break;
        case TYPE_DOUBLE:
            fprintf(fp, "\t%f\n", val.dbl);
            break;
        default:
            assert(0);
            break;
        }
        break;
    case SETD:
        fprintf(fp, "\t%f\n", val.dbl);
        break;
    default:
        fprintf(fp, "\n");
        break;
    }
}


sel_reg::sel_reg()
{
    for (int i = 0; i < (int)sizeof(regs); i++)
    {
        regs[i] = i;
    }
}

int sel_reg::get(const char* name)
{
    auto it = names.find(name);
    if (it != names.end())
    {
        int8_t* p = (int8_t*)memchr(regs, it->second, sizeof(regs));
        return used(p - regs);
    }
    else
    {
        int v = used(0);
        names.emplace(name, v);
        return v;
    }
}

int sel_reg::used(int id)
{
    int v = regs[id];
    if (id != sizeof(regs) - 1)
    {
        memmove(regs + id, regs + id + 1, sizeof(regs) - id - 1);
        regs[sizeof(regs) - 1] = v;
    }
    return v;
}
