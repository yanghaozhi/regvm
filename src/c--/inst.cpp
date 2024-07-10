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

inst::inst(const char* n, int i, int r, int e, int8_t* a) :
    bytes(4), id(i), reg(r), ex(e), name(n)
{
    memcpy(args, a, sizeof(args));
}

inst::inst(const char* n, int i, int r, int e, uv v) :
    bytes(2), id(i), reg(r), ex(e), name(n), val(v)
{
    switch (ex)
    {
    case TYPE_SIGNED:
        if (-32768 <= v.sint && v.sint <= 32767)
        {
            id = CODE_SETS;
            name = "SETS";
            bytes += 2;
        }
        else if (-2147483648 <= v.sint && v.sint <= 2147483647)
        {
            id = CODE_SETI;
            name = "SETI";
            bytes += 4;
        }
        else
        {
            id = CODE_SETL;
            name = "SETL";
            bytes += 8;
        }
        break;
    case TYPE_UNSIGNED:
        if (v.uint <= 0xFFFF)
        {
            id = CODE_SETS;
            name = "SETS";
            bytes += 2;
        }
        else if (v.uint <= 0xFFFFFFFF)
        {
            id = CODE_SETI;
            name = "SETI";
            bytes += 4;
        }
        else
        {
            id = CODE_SETL;
            name = "SETL";
            bytes += 8;
        }
        break;
    case TYPE_DOUBLE:
        id = CODE_SETL;
        name = "SETL";
        bytes += 8;
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
