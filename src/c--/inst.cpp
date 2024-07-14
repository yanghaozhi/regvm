#include "inst.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <string>


sel_reg         regs;

inst::inst(const char* n, int i, int r, int e) :
    bytes(2), id(i), reg(r), ex(e), name(n)
{
}

inst::inst(const char* n, int i, int r, const std::string_view& v) :
    bytes(10), id(CODE_SETC), reg(r), ex(TYPE_STRING), name("SETC"), str(v)
{
}

inst::inst(const char* n, int i, int r, int e, const std::vector<int>& a) :
    bytes(4), id(i), reg(r), ex(e), name(n), args(a)
{
    while (args.size() < 4)
    {
        args.emplace_back(0);
    }
}

inst::inst(const char* n, int i, int r, int e, uv v) :
    bytes(2), id(i), reg(r), ex(e), name(n), val(v)
{
    if (ex != TYPE_ADDR)
    {
        recalc();
    }
}

#define SIZE_SETS   2
#define SIZE_SETI   4
#define SIZE_SETL   8
#define CHANGE_CODE(x) id = CODE_##x; name = #x; bytes += SIZE_##x;
void inst::recalc()
{
    switch (ex)
    {
    case TYPE_SIGNED:
        if (-32768 <= val.sint && val.sint <= 32767)
        {
            CHANGE_CODE(SETS);
        }
        else if (-2147483648 <= val.sint && val.sint <= 2147483647)
        {
            CHANGE_CODE(SETI);
        }
        else
        {
            CHANGE_CODE(SETL);
        }
        break;
    case TYPE_UNSIGNED:
        if (val.uint <= 0xFFFF)
        {
            CHANGE_CODE(SETS);
        }
        else if (val.uint <= 0xFFFFFFFF)
        {
            CHANGE_CODE(SETI);
        }
        else
        {
            CHANGE_CODE(SETL);
        }
        break;
    case TYPE_DOUBLE:
        CHANGE_CODE(SETL);
        break;
    case TYPE_ADDR:
        CHANGE_CODE(SETI);
        break;
    default:
        break;
    }
}
#undef CHANGE_CODE
#undef SIZE_SETS
#undef SIZE_SETI
#undef SIZE_SETL


void inst::print(FILE* fp)
{
    fprintf(fp, "%s\t%02X\t%d\t%d\t%ld\t%s\n", name, id, reg, ex, val.sint, std::string(str).c_str());
}

void inst::print_bin(FILE* fp)
{
    switch (id)
    {
    case CODE_SETC:
        id = CODE_SETL;
        val.uint = (uintptr_t)str.c_str();
        break;
    case CODE_SETD:
        id = CODE_SETL;
        break;
    default:
        break;
    }

    code_t c;
    c.id = id;
    c.ex = ex;
    c.reg = reg;

    fwrite(&c, sizeof(code_t), 1, fp);

    switch (id)
    {
    case CODE_SETS:
        fwrite(&val.uint, 2, 1, fp);
        break;
    case CODE_SETI:
        fwrite(&val.uint, 4, 1, fp);
        break;
    case CODE_SETL:
        fwrite(&val.uint, 8, 1, fp);
        break;
    default:
        if (id >= 128)
        {
            union extend_args
            {
                uint16_t        v;
                struct
                {
                    uint16_t    a1 : 4;
                    uint16_t    a2 : 4;
                    uint16_t    a3 : 4;
                    uint16_t    a4 : 4;
                };
            }   ea;
            ea.a1 = args[0];
            ea.a2 = args[1];
            ea.a3 = args[2];
            ea.a4 = args[3];
            fwrite(&ea.v, sizeof(ea.v), 1, fp);
        }
        break;
    }
}

void inst::print_txt(FILE* fp)
{
    fprintf(fp, "%s\t%d\t%d", name, reg, ex);
    switch (id)
    {
    case CODE_SETC:
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
        case TYPE_ADDR:
            fprintf(fp, "\t%X\n", (uint32_t)val.uint);
            break;
        default:
            assert(0);
            break;
        }
        break;
    case CODE_SETD:
        fprintf(fp, "\t%f\n", val.dbl);
        break;
    default:
        if (id >= 128)
        {
            for (auto v : args)
            {
                fprintf(fp, "\t%d", v);
            }
        }
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
        return active(it->second);
    }
    else
    {
        int v = used(0);
        names.emplace(name, v);
        return v;
    }
}

int sel_reg::tmp(void)
{
    return regs[0];
}

int sel_reg::active(int reg_id)
{
    int8_t* p = (int8_t*)memchr(regs, reg_id, sizeof(regs));
    return used(p - regs);
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
