#include "inst.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <string>
#include <limits>


bool instv<CODE_JUMP>::scan(const char* str)
{
    return sscanf(str, "%d", &offset);
}

void instv<CODE_JUMP>::print(FILE* fp)
{
    fprintf(fp, "%8s\t%02X\t%d\n", name, id, offset);
}

void instv<CODE_JUMP>::print_bin(FILE* fp)
{
    code_t code;
    code.id = CODE_JUMP;
    code.a3 = offset;
    fwrite(&code, sizeof(code_t), 1, fp);
}

void instv<CODE_JUMP>::print_asm(FILE* fp)
{
    fprintf(fp, "%8s\t%d\n", name, offset);
}


instv<CODE_SET>::instv(const char* n, int r, const std::string_view& v) : inst(CODE_SET, n)
    ,reg(r), type(TYPE_STRING), str(v)
{
    c = set_datas((uintptr_t)str.c_str(), 1);
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

    bool r = false;
    switch (type)
    {
#define SCAN_V(k, f, t)                 \
    case k:                             \
        ex.t = f(p, &e, 10);            \
        c = set_datas(ex.t, 1);         \
        return r;
        SCAN_V(TYPE_SIGNED, strtoll, sint);
        SCAN_V(TYPE_UNSIGNED, strtoull, uint);
        SCAN_V(TYPE_DOUBLE, strtod, dbl);
#undef SCAN_V
    case TYPE_STRING:
        str = p;
        c = set_datas((uintptr_t)str.c_str(), 1);
        return true;
    default:
        return false;
    }
}

void instv<CODE_SET>::print(FILE* fp)
{
    switch (type)
    {
    case TYPE_SIGNED:
        fprintf(fp, "%8s\t%02X\t%d\t%d\t%ld\n", name, id, reg, type, ex.sint);
        return;
    case TYPE_UNSIGNED:
        fprintf(fp, "%8s\t%02X\t%d\t%d\t%lu\n", name, id, reg, type, ex.uint);
        return;
    case TYPE_DOUBLE:
        fprintf(fp, "%8s\t%02X\t%d\t%d\t%f\n", name, id, reg, type, ex.dbl);
        return;
    case TYPE_STRING:
        fprintf(fp, "%8s\t%02X\t%d\t%d\t%s\n", name, id, reg, type, str.c_str());
        return;
    default:
        assert(0);
        break;
    }
}

void instv<CODE_SET>::print_bin(FILE* fp)
{
    code_t code;
    code.a = reg;
    code.b = type;
    code.c = c;
    fwrite(&code, sizeof(code_t), 1, fp);

    for (auto& it : datas)
    {
        code.id = CODE_DATA;
        code.a3 = it;
        fwrite(&code, sizeof(code_t), 1, fp);
    }
}

void instv<CODE_SET>::print_asm(FILE* fp)
{
    switch (type)
    {
    case TYPE_SIGNED:
        fprintf(fp, "# %lld\n", (long long)ex.sint);
        break;
    case TYPE_UNSIGNED:
        fprintf(fp, "# %llu\n", (unsigned long long)ex.uint);
        break;
    case TYPE_DOUBLE:
        fprintf(fp, "# %.17g\n", ex.dbl);
        break;
    case TYPE_STRING:
        fprintf(fp, "# %s\n", str.c_str());
        break;
    default:
        break;
    }

    fprintf(fp, "%8s\t%d\t%d\t%d\n", name, reg, type, c);
    for (auto& it : datas)
    {
        fprintf(fp, "DATA    \t%d\n", it);
    }
}


inst::inst(int i, const char* n) : id(i), name(n)
{
}

int instex::set_datas(uint64_t v, int header)
{
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

