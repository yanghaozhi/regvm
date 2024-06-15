#include "source.h"

#include <code.h>

using namespace vasm;

source::~source()
{
    fclose(fp);
}

bool source::open(const char* name)
{
    file = name;

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
    fp = fopen(name, "r");
    return fp != NULL;
}

bool source::scan(void)
{
    fseek(fp, 0, SEEK_SET);

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


    char buf[1024];
    char data[1024];

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        cur_line += 1;
        id.v = 0;
        if (buf[0] == '#')
        {
            //comment(buf, size);
            continue;
        }

        buf[sizeof(buf) - 1] = '\0';
        data[0] = '\0';

        sscanf(buf, "%7s %d %d %1024[^\n]", id.s, &reg, &ex, data);
        data[sizeof(data) - 1] = '\0';

        memset(&inst, 0, sizeof(inst));

        inst.code.ex = ex;
        inst.code.reg = reg;
        auto it = ids.find(id.s);
        if (it != ids.end())
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
            if (id.v == 0x43544553) //SETC
            {
                setc(inst.code, (intptr_t*)(&inst.code + 1), data);
            }
            else
            {
                printf("\e[31m --- 0x%lX : %s \e[0m\n", id.v, id.s);
                continue;
            }
        }

        int b = line(&inst.code, sizeof(inst), buf);
        if (b == 0)
        {
            printf("\e[31m --- scan ERROR : %s\e[0m\n", buf);
            return false;
        }
    };

    return true;
}
