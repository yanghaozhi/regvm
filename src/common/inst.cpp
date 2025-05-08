#include "inst.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <format>
#include <iostream>

#include <string>
#include <limits>

#include <log.h>



static void data_print(FILE* fp, int v)
{
    fprintf(fp, "DATA    \t0x%06X\n", v);
}

static void data_print(std::ostream& out, int v)
{
    //fprintf(fp, "DATA    \t0x%06X\n", v);
    PRINTF(out, "DATA    \t0x{:06X}\n", v);
}

static void data_print_bin(FILE* fp, int v)
{
    code_t code;
    code.id = CODE_DATA;
    code.a3 = v;
    fwrite(&code, sizeof(code_t), 1, fp);
}

static void data_print_bin(std::ostream& out, int v)
{
    code_t code;
    code.id = CODE_DATA;
    code.a3 = v;
    WRITEC(out, code);
}


bool instj::scan(const char* src)
{
    return sscanf(src, "%d %d %d %d", &a, &b, &c, &offset) == 4;
}

void instj::print(std::ostream& out) const
{
    PRINTF(out, "{:<8} {:02X}\t{}\t{}\t{}\n", name, id, a, b, c);
    //fprintf(fp, "%-8s %02X\t%d\t%d\t%d\n", name, id, a, b, c);
    data_print(out, offset);
}

void instj::print_bin(std::ostream& out) const
{
    code_t code;
    code.id = id;
    code.a = a;
    code.b = b;
    code.c = c;
    //fwrite(&code, sizeof(code_t), 1, fp);
    WRITEC(out, code);
    data_print_bin(out, offset);
}

void instj::print_asm(std::ostream& out) const
{
    //fprintf(fp, "%-8s %d\t%d\t%d\t%d\n", name, a, b, c, offset);
    PRINTF(out, "{:<8} {}\t{}\t{}\t{}\n", name, a, b, c, offset);
}


bool instv<CODE_JUMP>::scan(const char* src)
{
    return sscanf(src, "%d", &offset);
}

void instv<CODE_JUMP>::print(std::ostream& out) const
{
    //fprintf(fp, "%-8s %02X\t%d\n", name, id, offset);
    PRINTF(out, "{:<8} {:02X}\t{}\n", name, id, offset);
}

void instv<CODE_JUMP>::print_bin(std::ostream& out) const
{
    code_t code;
    code.id = CODE_JUMP;
    code.a3 = offset;
    WRITEC(out, code);
}

void instv<CODE_JUMP>::print_asm(std::ostream& out) const
{
    //fprintf(fp, "%-8s %d\n", name, offset);
    PRINTF(out, "{:<8} {}\n", name, offset);
}


instv<CODE_SET>::instv(const char* n, int r, const std::string_view& v) : inst(CODE_SET, n)
    ,reg(r), type(TYPE_STRING), str(v)
{
    ex.str = str.c_str();
    c = set_datas((uintptr_t)ex.str, 1);
}

instv<CODE_SET>::instv(const char* n, int r, const char* v) : inst(CODE_SET, n)
    ,reg(r), type(TYPE_STRING)
{
    ex.str = v;
    c = set_datas((uintptr_t)ex.str, 1);
}

instv<CODE_SET>::instv(const char* n, int r, int t, uint64_t v) : inst(CODE_SET, n)
    ,reg(r), type(t)
{
    ex.uint = v;
    c = set_datas(ex.uint, 1);
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
        SCAN_V(TYPE_ADDR, uint, strtoll, 10);
#undef SCAN_V
    case TYPE_STRING:
        while ((*p == ' ') || (*p == '\t'))
        {
            p++;
        }
        return change_str(p);
    default:
        return false;
    }
}

void instv<CODE_SET>::print(std::ostream& out) const
{
    switch (type)
    {
    case TYPE_SIGNED:
		PRINTF(out, "# {}\n{:<8} {:02X}\t{}\t{}\t{}\n", ex.sint, name, id, reg, type, c);
        break;
    case TYPE_UNSIGNED:
		PRINTF(out, "# {}\n{:<8} {:02X}\t{}\t{}\t{}\n", ex.uint, name, id, reg, type, c);
        break;
    case TYPE_DOUBLE:
		PRINTF(out, "# {:.17g}\n{:<8} {:02X}\t{}\t{}\t{}\n", ex.dbl, name, id, reg, type, c);
        break;
    case TYPE_STRING:
		PRINTF(out, "# {}\n{:<8} {:02X}\t{}\t{}\t{}\n", ex.str, name, id, reg, type, c);
        break;
    case TYPE_ADDR:
		PRINTF(out, "# {}\n{:<8} {:02X}\t{}\t{}\t{}\n", ex.uint, name, id, reg, type, c);
        return;
    default:
        LOGW("Unknown type : %d", type);
        break;
    }

    for (auto& it : datas)
    {
        data_print(out, it);
    }
}

void instv<CODE_SET>::print_bin(std::ostream& out) const
{
    code_t code;
    code.id = CODE_SET;
    code.a = reg;
    code.b = type;
    code.c = c;
    WRITEC(out, code);

    for (auto& it : datas)
    {
        data_print_bin(out, it);
    }
}

void instv<CODE_SET>::print_asm(std::ostream& out) const
{
    switch (type)
    {
    case TYPE_SIGNED:
		PRINTF(out, "{:<8} {}\t{}\t{}\n", name, reg, type, ex.sint);
        return;
    case TYPE_UNSIGNED:
		PRINTF(out, "{:<8} {}\t{}\t{}\n", name, reg, type, ex.uint);
        return;
    case TYPE_DOUBLE:
		PRINTF(out, "{:<8} {}\t{}\t{}\n", name, reg, type, ex.dbl);
        return;
    case TYPE_STRING:
		PRINTF(out, "{:<8} {}\t{}\t{}\n", name, reg, type, ex.str);
        return;
    case TYPE_ADDR:
		PRINTF(out, "{:<8} {}\t{}\t{}\n", name, reg, type, ex.uint);
        return;
    default:
        LOGW("Unknown type : %d", type);
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

bool instv<CODE_ECHO>::scan(const char* src)
{
    char* e = NULL;
    const char* p = src;
    while ((p != NULL) && (*p != '\n') && (*p != '\r') && (*p != '\n'))
    {
        args.push_back(strtol(p, &e, 10));
        p = e;
    }
    return true;
}

int instv<CODE_ECHO>::count(void) const
{
    int s = args.size() - 2;
    return (s <= 0) ? 1 : (s / 3) + (int)(bool)(s % 3);
}

template <typename F> inline void cmd_args(const std::vector<int>& args, const int skip, F func)
{
    const int size = args.size() - skip;
    for (int i = 0; i < size / 3; i++)
    {
        const int base = i * 3 + skip;
        func(args[base + 0], args[base + 1], args[base + 2]);
    }
    switch (size % 3)
    {
    case 1:
        func(args[args.size() - 1], 0, 0);
        break;
    case 2:
        func(args[args.size() - 2], args[args.size() - 1], 0);
        break;
    }
}

void instv<CODE_ECHO>::print(std::ostream& out) const
{
	PRINTF(out, "{:<8} {:02X}\t{}", name, id, args.size());
    switch (args.size())
    {
    case 0:
        LOGW("Should NOT echo with 0 args !!!");
		PRINTF(out, "\t{}\t{}\n", 0, 0);
        break;
    case 1:
		PRINTF(out, "\t{}\t{}\n", args[0], 0);
        break;
    case 2:
		PRINTF(out, "\t{}\t{}\n", args[0], args[1]);
        break;
    default:
		PRINTF(out, "\t{}\t{}\n", args[0], args[1]);
        cmd_args(args, 2, [&out](int a, int b, int c)
            {
				PRINTF(out, "DATA     \t{}\t{}\t{}\n", a, b, c);
            });
        break;
    }
}

void instv<CODE_ECHO>::print_bin(std::ostream& out) const
{
    code_t code = {0};
    code.id = CODE_ECHO;
    code.a = args.size();

    switch (args.size())
    {
    case 0:
        LOGW("Should NOT echo with 0 args !!!");
        code.b = 0;
        code.c = 0;
        WRITEC(out, code);
        break;
    case 2:
        code.c = args[1];
        [[fallthrough]];
    case 1:
        code.b = args[0];
        WRITEC(out, code);
        break;
    default:
        code.b = args[0];
        code.c = args[1];
        WRITEC(out, code);
        cmd_args(args, 2, [&out, &code](int a, int b, int c)
            {
                code.id = CODE_DATA;
                code.a = a;
                code.b = b;
                code.c = c;
				WRITEC(out, code);
            });
        break;
    }
}

void instv<CODE_ECHO>::print_asm(std::ostream& out) const
{
	PRINTF(out, "{:<8} {}", name, args.size());
    for (auto& it : args)
    {
		PRINTF(out, "\t{}", it);
    }
	PRINTF(out, "\n");
    if (args.size() == 0)
    {
        LOGW("Should NOT echo with 0 args !!!");
    }
}

bool instv<CODE_CALL>::scan(const char* src)
{
    return sscanf(src, "%d %d", &info, &func) == 2;
}

void instv<CODE_CALL>::print(std::ostream& out) const
{
	PRINTF(out, "# func id : {}\n", func);
    if (func > 0x7FFF)
    {
		PRINTF(out, "{:<8} {:02X}\t{}\t{}\n", name, id, info, (func & 0xFF) + 0xFF00);
        data_print(out, func >> 8);
    }
    else
    {
		PRINTF(out, "{:<8} {:02X}\t{}\t{}\n", name, id, info, func);
    }
}

void instv<CODE_CALL>::print_bin(std::ostream& out) const
{
    code_t code;
    code.id = CODE_CALL;
    code.a = info;
    code.b2 = (func > 0x7FFF) ? (func & 0xFF) + 0xFF00 : func;
    WRITEC(out, code);

    if (func > 0x7FFF)
    {
        data_print_bin(out, func >> 8);
    }
}

void instv<CODE_CALL>::print_asm(std::ostream& out) const
{
	PRINTF(out, "{:<8} {}\t{}\t{}\n", name, id, info, func);
}


