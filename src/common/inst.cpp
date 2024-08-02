#include "inst.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <string>
#include <limits>


static void data_print(FILE* fp, int v)
{
    fprintf(fp, "DATA    \t0x%06X\n", v);
}

static void data_print_bin(FILE* fp, int v)
{
    code_t code;
    code.id = CODE_DATA;
    code.a3 = v;
    fwrite(&code, sizeof(code_t), 1, fp);
}

bool instj::scan(const char* str)
{
    return sscanf(str, "%d %d %d", &a, &b, &offset) == 3;
}

int instj::count(void) const
{
    return ((-127 <= offset) && (offset <= 127)) ? 1 : 2;
}

void instj::print(FILE* fp) const
{
    if ((-127 <= offset) && (offset <= 127))
    {
        fprintf(fp, "%-8s %02X\t%d\t%d\t%d\n", name, id, a, b, offset);
    }
    else
    {
        fprintf(fp, "%-8s %02X\t%d\t%d\t0\n", name, id, a, b);
        data_print(fp, offset);
    }
}

void instj::print_bin(FILE* fp) const
{
    code_t code;
    code.id = id;
    code.a = a;
    code.b = b;
    if ((-127 <= offset) && (offset <= 127))
    {
        code.c = offset;
        fwrite(&code, sizeof(code_t), 1, fp);
    }
    else
    {
        code.c = 0;
        fwrite(&code, sizeof(code_t), 1, fp);
        data_print_bin(fp, offset);
    }
}

void instj::print_asm(FILE* fp) const
{
    fprintf(fp, "%-8s %d\t%d\t%d\n", name, a, b, offset);
}


bool instv<CODE_JUMP>::scan(const char* str)
{
    return sscanf(str, "%d", &offset);
}

void instv<CODE_JUMP>::print(FILE* fp) const
{
    fprintf(fp, "%-8s %02X\t%d\n", name, id, offset);
}

void instv<CODE_JUMP>::print_bin(FILE* fp) const
{
    code_t code;
    code.id = CODE_JUMP;
    code.a3 = offset;
    fwrite(&code, sizeof(code_t), 1, fp);
}

void instv<CODE_JUMP>::print_asm(FILE* fp) const
{
    fprintf(fp, "%-8s %d\n", name, offset);
}


instv<CODE_SET>::instv(const char* n, int r, const char* v) : inst(CODE_SET, n)
    ,reg(r), type(TYPE_STRING)
{
    ex.str = v;
    c = set_datas((uintptr_t)ex.str, 1);
}

instv<CODE_SET>::instv(const char* n, int r, uint64_t v) : inst(CODE_SET, n)
    ,reg(r), type(TYPE_UNSIGNED)
{
    ex.uint = v;
    c = set_datas(ex.uint, 1);
}

instv<CODE_SET>::instv(const char* n, int r, int64_t v) : inst(CODE_SET, n)
    ,reg(r), type(TYPE_SIGNED)
{
    ex.sint = v;
    c = set_datas(ex.sint, 1);
}

instv<CODE_SET>::instv(const char* n, int r, double v) : inst(CODE_SET, n)
    ,reg(r), type(TYPE_DOUBLE)
{
    ex.dbl = v;
    c = set_datas(ex.uint, 1);
}

bool instv<CODE_SET>::scan(const char* s)
{
    char* e = NULL;
    const char* p = s;
    reg = strtol(p, &e, 10);
    p = e;
    type = strtol(p, &e, 10);
    p = e;

    switch (type)
    {
#define SCAN_V(k, t, f, ...)            \
    case k:                             \
        ex.t = f(p, &e, ##__VA_ARGS__); \
        c = set_datas(ex.uint, 1);      \
        return true;
        SCAN_V(TYPE_SIGNED, sint, strtoll, 10);
        SCAN_V(TYPE_UNSIGNED, uint, strtoull, 10);
        SCAN_V(TYPE_DOUBLE, dbl, strtod);
#undef SCAN_V
    case TYPE_STRING:
        return change_str(p);
    default:
        return false;
    }
}

void instv<CODE_SET>::print(FILE* fp) const
{
    switch (type)
    {
    case TYPE_SIGNED:
        fprintf(fp, "# %lld\n", (long long)ex.sint);
        fprintf(fp, "%-8s %02X\t%d\t%d\t%ld\n", name, id, reg, type, ex.sint);
        break;
    case TYPE_UNSIGNED:
        fprintf(fp, "# %llu\n", (unsigned long long)ex.uint);
        fprintf(fp, "%-8s %02X\t%d\t%d\t%lu\n", name, id, reg, type, ex.uint);
        break;
    case TYPE_DOUBLE:
        fprintf(fp, "# %.17g\n", ex.dbl);
        fprintf(fp, "%-8s %02X\t%d\t%d\t%f\n", name, id, reg, type, ex.dbl);
        break;
    case TYPE_STRING:
        fprintf(fp, "# %s\n", ex.str);
        fprintf(fp, "%-8s %02X\t%d\t%d\t%s\n", name, id, reg, type, ex.str);
        break;
    default:
        break;
    }

    fprintf(fp, "%-8s %d\t%d\t%d\n", name, reg, type, c);
    for (auto& it : datas)
    {
        data_print(fp, it);
    }
}

void instv<CODE_SET>::print_bin(FILE* fp) const
{
    code_t code;
    code.a = reg;
    code.b = type;
    code.c = c;
    fwrite(&code, sizeof(code_t), 1, fp);

    for (auto& it : datas)
    {
        data_print_bin(fp, it);
    }
}

void instv<CODE_SET>::print_asm(FILE* fp) const
{
    switch (type)
    {
    case TYPE_SIGNED:
        fprintf(fp, "%-8s %d\t%d\t%ld\n", name, reg, type, ex.sint);
        return;
    case TYPE_UNSIGNED:
        fprintf(fp, "%-8s %d\t%d\t%lu\n", name, reg, type, ex.uint);
        return;
    case TYPE_DOUBLE:
        fprintf(fp, "%-8s %d\t%d\t%f\n", name, reg, type, ex.dbl);
        return;
    case TYPE_STRING:
        fprintf(fp, "%-8s %d\t%d\t%s\n", name, reg, type, ex.str);
        return;
    default:
        assert(0);
        break;
    }
}

bool instv<CODE_SET>::change_str(const char* n)
{
    ex.str = n;
    c = set_datas((uintptr_t)ex.str, 1);
    return true;
}

inst::inst(int i, const char* n) : id(i), name(n)
{
}

int instex::set_datas(uint64_t v, int header)
{
    datas.clear();

    int h = 0;
    switch (header)
    {
    case 3:
        h = v & 0xFFFFFF;
        v >>= 24;
        break;
    case 2:
        h = v & 0xFFFF;
        v >>= 16;
        break;
    case 1:
        h = v & 0xFF;
        v >>= 8;
        break;
    default:
        return -1;
    }

    if (v == 0) return h;

    datas.push_back(v & 0xFFFFFF);
    v >>= 24;
    if (v == 0) return h;

    datas.push_back(v & 0xFFFFFF);
    v >>= 24;
    if (v == 0) return h;

    datas.push_back(v & 0xFFFFFF);

    return h;
}

