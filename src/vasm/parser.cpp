#include "parser.h"

#include <fcntl.h>
#include <sys/mman.h>

#include <code.h>

using namespace vasm;

parser::~parser()
{
}

bool parser::finish()
{
    if (fd >= 0)
    {
        munmap(data, size);
        close(fd);
    }
    //return scan(comment2, setc2, line2);
    return true;
}

bool parser::open(const char* name)
{
    file = name;
    int fd = ::open(name, O_RDONLY);
    struct stat st;
    fstat(fd, &st);
    auto d = (char*)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    return open(d, st.st_size);
}

bool parser::open(char* d, int64_t s)
{
    data = d;
    size = s;
    if (ids.empty() == true)
    {
#define SET_KEY(k) ids.emplace(#k, CODE_##k);
        SET_KEY(NOP);
        SET_KEY(TRAP);
        SET_KEY(SETS);
        SET_KEY(SETI);
        SET_KEY(SETL);
        SET_KEY(MOVE);
        SET_KEY(CLEAR);
        SET_KEY(LOAD);
        SET_KEY(STORE);
        SET_KEY(BLOCK);
        SET_KEY(CALL);
        SET_KEY(RET);
        SET_KEY(CMD);
        SET_KEY(INC);
        SET_KEY(DEC);
        SET_KEY(ADD);
        SET_KEY(SUB);
        SET_KEY(MUL);
        SET_KEY(DIV);
        SET_KEY(AND);
        SET_KEY(OR);
        SET_KEY(XOR);
        SET_KEY(CONV);
        SET_KEY(CHG);
        SET_KEY(JUMP);
        SET_KEY(JZ);
        SET_KEY(JNZ);
        SET_KEY(JG);
        SET_KEY(JL);
        SET_KEY(JNG);
        SET_KEY(JNL);
        SET_KEY(EXIT);
#undef SET_KEY
    }
    return true;
}

bool parser::pass::scan(void)
{
    union 
    {
        char        data[10];
        code_t      code;
    }               inst;

    union
    {
        char        s[8];
        uint64_t    v;
    }               id;
    int             ex;
    int             reg;

    before();

    char* end = NULL;
    const char* buf = NULL;
    char data[1024];

    while ((buf = src.next_line(&end)) != NULL)
    {
        cur_line += 1;
        id.v = 0;
        switch (buf[0])
        {
        case '#':
            comment(buf);
            continue;
        case '\0':
            continue;
        default:
            break;
        }

        data[0] = '\0';

        sscanf(buf, "%7s %d %d %1024[^\n]", id.s, &reg, &ex, data);
        data[sizeof(data) - 1] = '\0';

        memset(&inst, 0, sizeof(inst));

        inst.code.ex = ex;
        inst.code.reg = reg;
        auto it = src.ids.find(id.s);
        if (it != src.ids.end())
        {
            inst.code.id = it->second;

            switch (inst.code.id)
            {
            case CODE_SETS:
                {
                    int v = 0;
                    sscanf(data, "%d", &v);
                    *(int16_t*)(&inst.code + 1) = v;
                }
                break;
            case CODE_SETI:
                sscanf(data, "%d", (int32_t*)(&inst.code + 1));
                break;
            case CODE_SETL:
                inst.code.id = CODE_SETL;
                sscanf(data, "%ld", (int64_t*)(&inst.code + 1));
                break;
            default:
                break;
            }
        }
        else
        {
            switch (id.v)
            {
            case 0x44544553:    //SETD
                inst.code.id = CODE_SETL;
                sscanf(data, "%lf", (double*)(&inst.code + 1));
                break;
            case 0x43544553:    //SETC
                if (setc(inst.code, (intptr_t*)(&inst.code + 1), data) == false)
                {
                    ERROR("\e[31m --- setc ERROR : {}\e[0m\n", buf);
                    return false;
                }
                break;
            default:
                ERROR("\e[31m --- 0x{} : {} \e[0m\n", id.v, id.s);
                continue;
            }
        }

        if (line(&inst.code, sizeof(inst), buf) == false)
        {
            ERROR("\e[31m --- scan ERROR : {}\e[0m\n", buf);
            return false;
        }
    };

    after();

    return true;
}

const char* parser::next_line(char** end) const
{
    const char k = '\n';
    char* s = data;
    if (*end != NULL)
    {
        if (*end - data >= size)
        {
            return NULL;
        }

        **end = k;
        s = *end + 1;
    }
    char* p = strchr(s, k);
    if (p == NULL)
    {
        *end = data + size;
        return s;
    }
    else
    {
        *p = k;
        *end = p;
        return s;
    }
}
