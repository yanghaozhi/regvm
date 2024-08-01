#include "parser.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <code.h>

using namespace vasm;

parser::~parser()
{
    for (auto& it : insts)
    {
        delete it;
    }
}

//bool parser::finish(FILE* fp)
//{
//    if (fd >= 0)
//    {
//        munmap((void*)data, size);
//        close(fd);
//    }
//    return true;
//}
//
//bool parser::open(const char* name)
//{
//    file = name;
//    int fd = ::open(name, O_RDONLY);
//    struct stat st;
//    fstat(fd, &st);
//    auto d = (char*)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
//    return open(d, st.st_size);
//}

bool parser::open(char* d, int64_t s)
{
    size = s;
    if (ids.empty() == true)
    {
#define SET_KEY(k) ids.emplace(#k, new instv<CODE_##k>(#k));
        SET_KEY(NOP);
        SET_KEY(DATA);
        SET_KEY(MOVE);
        SET_KEY(CHG);
        SET_KEY(CMP);
        SET_KEY(TYPE);
        SET_KEY(INC);
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
        SET_KEY(SHIFT);
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
            continue;
        case '\n':
        case '\r':
            lineno += 1;
            [[fallthrough]];
        case '\0':
            continue;
        default:
            break;
        }

        e = strchr(p, ' ');
        if (e == NULL)
        {
            return true;
        }

        std::string_view k(p, e - p);
        auto it = ids.find(k);
        if (it == ids.end())
        {
            LOGE("%d : find INVALID inst : %s !!!", lineno, std::string(k).c_str());
            return false;
        }

        if (line(p, it->second) == false)
        {
            LOGE("%d : parse line ERROR !!!", lineno);
            return false;
        }
    };

    return true;
}

bool parser::line(const char* str, inst* orig)
{
    insts.emplace_back(orig->copy());
    if (insts.back()->scan(str) == false)
    {
        LOGE("%d : scan %s ERROR", lineno, insts.back()->name);
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

