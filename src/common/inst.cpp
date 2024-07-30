#include "inst.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <string>
#include <limits>


instv<CODE_JUMP>::instv(const char* n, int w) : inst(print, print_bin, print_asm)
{
    ex.sint = w;
}

void instv<CODE_JUMP>::print(inst* self, FILE* fp)
{
    fprintf(fp, "%8s\t%02X\t%d\n", self->name, self->id, (int)self->ex.sint);
}

void instv<CODE_JUMP>::print_bin(inst* self, FILE* fp)
{
}

void instv<CODE_JUMP>::print_asm(inst* self, FILE* fp)
{
}

inst::inst(print_op f1, print_op f2, print_op f3)
{
    ops[0] = f1;
    ops[1] = f2;
    ops[2] = f3;
}

//inst::inst(const char* n, int i, int _a, int _b, int _c) :
//    id(i), a(_a), b(_b), c(_c), name(n)
//{
//}
//
//inst::inst(const char* n, int i, int r, const std::string_view& v) :
//    type(TYPE_STRING), id(CODE_SET), a(r), b(TYPE_STRING), name("SET"), str(v)
//{
//    recalc();
//}
//
//inst::inst(const char* n, int i, int r, uint64_t v) :
//    type(TYPE_UNSIGNED), id(i), a(r), b(TYPE_UNSIGNED), name(n)
//{
//    ex.uint = v;
//    recalc();
//}
//
//inst::inst(const char* n, int i, int a3) :
//    id(i), name(n)
//{
//    ex.sint = a3;
//}

void inst::recalc()
{
    datas.clear();

    switch (id)
    {
    case CODE_SET:
        switch (type)
        {
        case TYPE_SIGNED:
        case TYPE_UNSIGNED:
        case TYPE_DOUBLE:
            c = set_datas(ex.uint, 1);
            break;
        case TYPE_STRING:
            c = set_datas((uintptr_t)str.c_str(), 1);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    count = datas.size() + 1;
}

int inst::set_datas(uint64_t v, int header)
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

void inst::print(FILE* fp)
{
    ops[0](this, fp);
}

void inst::print_bin(FILE* fp)
{
    ops[1](this, fp);
}

void inst::print_asm(FILE* fp)
{
    ops[2](this, fp);
}

//void inst::print(FILE* fp)
//{
//    //switch (id)
//    //{
//    //case CODE_SET:
//    //    switch (type)
//    //    {
//    //    case TYPE_SIGNED:
//    //        fprintf(fp, "%8s\t%02X\t%d\t%d\t%ld\n", name, id, a, b, ex.sint);
//    //        return;
//    //    case TYPE_UNSIGNED:
//    //        fprintf(fp, "%8s\t%02X\t%d\t%d\t%lu\n", name, id, a, b, ex.uint);
//    //        return;
//    //    case TYPE_DOUBLE:
//    //        fprintf(fp, "%8s\t%02X\t%d\t%d\t%f\n", name, id, a, b, ex.dbl);
//    //        return;
//    //    case TYPE_STRING:
//    //        fprintf(fp, "%8s\t%02X\t%d\t%d\t%s\n", name, id, a, b, str.c_str());
//    //        return;
//    //    default:
//    //        assert(0);
//    //        break;
//    //    }
//    //    break;
//    //default:
//    //    if (a == 0xFFFFFFFF)
//    //    {
//    //        fprintf(fp, "%8s\t%02X\t%ld\n", name, id, ex.sint);
//    //        return;
//    //    }
//    //    if (b == 0xFFFFFFFF)
//    //    {
//    //        fprintf(fp, "%8s\t%02X\t%d\n%ld\n", name, id, a, ex.sint);
//    //        return;
//    //    }
//    //    fprintf(fp, "%8s\t%02X\t%d\t%d\t%d\n", name, id, a, b, c);
//    //    break;
//    //}
//}
//
//void inst::print_bin(FILE* fp)
//{
//    ops[1](this, fp);
//    //code_t code;
//    //code.id = id;
//
//    //if (a == 0xFFFFFFFF)
//    //{
//    //    code.a3 = ex.sint;
//    //    fwrite(&code, sizeof(code_t), 1, fp);
//    //    return;
//    //}
//    //if (b == 0xFFFFFFFF)
//    //{
//    //    code.a2 = a;
//    //    code.b2 = ex.sint;
//    //    fwrite(&code, sizeof(code_t), 1, fp);
//    //    return;
//    //}
//
//    //code.a = a;
//    //code.b = b;
//    //code.c = c;
//    //fwrite(&code, sizeof(code_t), 1, fp);
//
//    //for (auto& it : datas)
//    //{
//    //    code.id = CODE_DATA;
//    //    code.a3 = it;
//    //    fwrite(&code, sizeof(code_t), 1, fp);
//    //}
//}
//
//void inst::print_asm(FILE* fp)
//{
//    if (a == 0xFFFFFFFF)
//    {
//        fprintf(fp, "%8s\t%ld\n", name, ex.sint);
//        return;
//    }
//    if (b == 0xFFFFFFFF)
//    {
//        fprintf(fp, "%8s\t%d\t%ld\n", name, a, ex.sint);
//        return;
//    }
//
//    switch (id)
//    {
//    case CODE_SET:
//        switch (type)
//        {
//        case TYPE_SIGNED:
//            fprintf(fp, "# %lld\n", (long long)ex.sint);
//            break;
//        case TYPE_UNSIGNED:
//            fprintf(fp, "# %llu\n", (unsigned long long)ex.uint);
//            break;
//        case TYPE_DOUBLE:
//            fprintf(fp, "# %.17g\n", ex.dbl);
//            break;
//        case TYPE_STRING:
//            fprintf(fp, "# %s\n", str.c_str());
//            break;
//        default:
//            break;
//        }
//        break;
//    default:
//        break;
//    }
//
//    fprintf(fp, "%8s\t%d\t%d\t%d\n", name, a, b, c);
//    for (auto& it : datas)
//    {
//        fprintf(fp, "DATA    \t%d\n", it);
//    }
//}
//
//
