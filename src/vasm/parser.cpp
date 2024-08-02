#include "parser.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
//#include <sys/mman.h>

#include <code.h>

using namespace vasm;

struct IDS
{
    IDS();
    ~IDS();

    std::unordered_map<std::string_view, inst* (*)(const char*)> insts;
};
static IDS  ids;

parser::parser()
{
}

parser::~parser()
{
    for (auto& it : insts)
    {
        delete it;
    }
}

int64_t parser::size(void) const
{
    int64_t s = 0;
    for (auto& it : insts)
    {
        s += it->count();
    }
    return s * 4;
}

bool parser::finish(FILE* fp, void (inst::*op)(FILE*) const)
{
    for (auto& it : insts)
    {
        (it->*op)(fp);
    }
    return true;
}

bool parser::go(const char* src)
{
    const char* e = NULL;

    const char* p = src;
    while ((p = next_token(p)) != NULL)
    {
        switch (*p)
        {
        case '#':
            e = strchr(p, '\n');
            if (e == NULL) return true;
            comment(p, e - p);
            p = e;
            continue;
        case '\n':
        case '\r':
            lineno += 1;
            p += 1;
            continue;
        case '\0':
            return true;
        default:
            break;
        }

        e = strchr(p, ' ');
        if (e == NULL)
        {
            return true;
        }

        std::string_view k(p, e - p);
        auto it = ids.insts.find(k);
        if (it == ids.insts.end())
        {
            LOGE("%d : find INVALID inst : %s !!!", lineno, std::string(k).c_str());
            return false;
        }

        if (line(next_token(e), it->second(it->first.data())) == false)
        {
            LOGE("%d : parse line ERROR !!!", lineno);
            return false;
        }

        p = strchr(e, '\n');
    };

    return true;
}

bool parser::line(const char* str, inst* code)
{
    insts.emplace_back(code);
    if (code->scan(str) == false)
    {
        LOGE("%d : scan %s ERROR", lineno, code->name);
        return false;
    }
    return true;
}

bool parser::comment(const char* line, int size)
{
    return true;
}

const char* parser::next_token(const char* src) const
{
    if (src == NULL) return NULL;

    const char* p = src;
    do
    {
        switch (*p)
        {
        case ' ':
        case '\t':
            continue;
        case '\n':
            return p;
        case 0:
            return NULL;
        default:
            return p;
        }
    } while (*p++ != '\0');
    return NULL;
}

IDS::IDS()
{
#define SET_KEY(k) insts.emplace(#k, &create_inst<CODE_##k>);
    SET_KEY(NOP);
    SET_KEY(DATA);
    SET_KEY(MOVE);
    SET_KEY(CHG);
    SET_KEY(CMP);
    SET_KEY(TYPE);
    SET_KEY(CALC);
    SET_KEY(ADD);
    SET_KEY(SUB);
    SET_KEY(MUL);
    SET_KEY(DIV);
    SET_KEY(MOD);
    SET_KEY(AND);
    SET_KEY(OR);
    SET_KEY(XOR);
    SET_KEY(SHL);
    SET_KEY(SHR);
    SET_KEY(JUMP);
    SET_KEY(JEQ);
    SET_KEY(JNE);
    SET_KEY(JGT);
    SET_KEY(JGE);
    SET_KEY(JLT);
    SET_KEY(JLE);
    SET_KEY(TRAP);
    SET_KEY(SET);
    SET_KEY(LOAD);
    SET_KEY(CLEAR);
    SET_KEY(STORE);
    SET_KEY(BLOCK);
    SET_KEY(CONV);
    SET_KEY(CALL);
    SET_KEY(RET);
    SET_KEY(ECHO);
    SET_KEY(SLEN);
    SET_KEY(LLEN);
    SET_KEY(LAT);
    SET_KEY(LSET);
    SET_KEY(LPUSH);
    SET_KEY(LPOP);
    SET_KEY(LERASE);
    SET_KEY(DLEN);
    SET_KEY(DGET);
    SET_KEY(DSET);
    SET_KEY(DDEL);
    SET_KEY(DHAS);
    SET_KEY(DITEMS);
    SET_KEY(EXIT);
#undef SET_KEY
}

IDS::~IDS()
{
}
