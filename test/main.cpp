#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <irq.h>
#include <code.h>
#include <regvm.h>
#include <debug.h>

#include <map>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

static int verbose = 0;
#define SHOW(fmt, ...)              \
    if (verbose > 0)                \
    {                               \
        printf(fmt, ##__VA_ARGS__); \
    }

void print_uvalue(int type, union regvm_uvalue uv)
{
    switch (type)
    {
    case TYPE_SIGNED:
        printf("%lld\n", (long long)uv.sint);
        break;
    case TYPE_UNSIGNED:
        printf("%llu\n", (unsigned long long)uv.uint);
        break;
    case TYPE_STRING:
        printf("%s\n", uv.str);
        break;
    case TYPE_DOUBLE:
        printf("%f\n", uv.dbl);
        break;
    default:
        printf("\n");
        break;
    }
}

struct dump_arg
{
    int     reg;
};

void dump_reg_info(void* arg, const struct regvm_reg_info* info)
{
    auto p = (struct dump_arg*)arg;

    switch ((intptr_t)info)
    {
    case 0:
        printf("\e[33m id\ttype\tref\tvar\tvalue \e[0m\n");
        break;
    case -1:
        break;
    default:
        if ((p->reg < 0) || (p->reg == info->id))
        {
            printf(" %d\t%d\t%d\t%p\t", info->id, info->type, info->ref, info->from);
            print_uvalue(info->type, info->value);
        }
        break;
    }
}

void dump_var_info(void* arg, const struct regvm_var_info* info)
{
    switch ((intptr_t)info)
    {
    case 0:
        printf("\e[32m type\treg\tref\tname @ func @ scope\tptr\tvalue\e[0m\n");
        break;
    case -1:
        break;
    default:
        printf(" %d\t%d\t%d\t%s @ %s @ %d\t%p\t", info->type, info->reg, info->ref, info->name, info->func, info->scope, info->var);
        print_uvalue(info->type, info->value);
        break;
    }
}

int64_t dump_trap_callback(struct regvm* vm, void*, int irq, code_t code, int offset, void* extra)
{
    struct dump_arg arg = {0};
    switch (code.ex)
    {
    case 0: //all regs
        arg.reg = -1;
        regvm_debug_reg_callback(vm, dump_reg_info, &arg);
        break;
    case 1: //1 arg
        arg.reg = code.reg;
        regvm_debug_reg_callback(vm, dump_reg_info, &arg);
        break;
    case 2:
        regvm_debug_var_callback(vm, dump_var_info, &arg);
        break;
    default:
        break;
    }
    return true;
}

int dump_error_callback(struct regvm* vm, void* arg, int irq, code_t code, int offset, void* extra)
{
    //printf("\e[31m %d - %s \e[0m\n", code, reason);
    return true;
}

class run
{
public:
    run() : vm(regvm_init())    {}
    virtual ~run()              {regvm_exit(vm);}

    struct regvm* vm            = NULL;

    virtual bool go(void)       = 0;
    virtual bool prepare(const char* file)
    {
        regvm_irq_set(vm, IRQ_TRAP, dump_trap_callback, NULL);
        return true;
    }
};

class txt : public run
{
public:
    FILE*           fp = NULL;
    uint32_t        lines   = 0;
    uint32_t        str_id  = 0;
    uint64_t        size    = 0;
    std::string     src;

    std::unordered_map<std::string, uint32_t>   string_table;
    std::unordered_map<std::string, int>        ids;


    virtual ~txt()  {fclose(fp);}
    virtual int line(const code_t* code, int max_bytes, const char* orig)   = 0;
    virtual bool prepare(const char* file)
    {
        run::prepare(NULL);
        src = file;

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
        fp = fopen(file, "r");
        return fp != NULL;
    }

    virtual void comment(const char* line, uint64_t bytes)
    {
        printf("\e[35m %s\e[0m", line);
    }

    virtual uint32_t setc(code_t& code, intptr_t* next, const char* str)
    {
        code.id = CODE_SETL;
        auto it = string_table.try_emplace(str, ++str_id).first;
        *next = (intptr_t)it->first.c_str();
        return it->second;
    }

    virtual bool go(void)
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


        char buf[1024];
        char data[1024];

        while (fgets(buf, sizeof(buf), fp) != NULL)
        {
            lines += 1;
            id.v = 0;
            if (buf[0] == '#')
            {
                comment(buf, size);
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
                printf("\e[31m --- run ERROR : %s\e[0m\n", buf);
                return false;
            }
            else
            {
                size += b;
            }
        };

        return true;
    }
};

class step : public txt
{
    virtual int line(const code_t* code, int max_bytes, const char* orig)
    {
        SHOW("--- ");
        for (int i = 0; i < max_bytes; i++)
        {
            SHOW(" %02X", ((unsigned char*)code)[i]);
        }
        SHOW(" : %02d : %s", code->id, orig);

        return regvm_exec_step(vm, code, max_bytes) << 1;
    }
};

class compile : public txt
{
public:
    int64_t        cur_label_id = 0;

    std::unordered_map<std::string, int64_t>        label_ids;
    std::map<uint64_t, int64_t>                     label_mod;  //mod -> id
    struct label_info
    {
        uint32_t    pos;
        uint32_t    line;
        uint16_t    label_len;
        uint16_t    file_len;
        char        data[];
    };
    std::unordered_map<int64_t, label_info*>        label_infos;

    int64_t label_id(const std::string& label)
    {
        auto it = label_ids.find(label);
        if (it != label_ids.end()) return it->second;
        return label_ids.emplace(label, ++cur_label_id).first->second;
    }

    bool find_label(const char* str, std::string& label)
    {
        char buf[256];
        if (sscanf(str, "#LABEL: %255[^\n]", buf) == 0) return false;
        buf[sizeof(buf) - 1] = '\0';
        label = buf;
        return true;
    }
    virtual uint32_t setc(code_t& code, intptr_t* next, const char* str)
    {
        std::string label;
        if (find_label(str, label) == true)
        {
            auto id = label_id(label);
            code.ex = 0x09;
            code.id = CODE_SETI;
            *(uint32_t*)next= (uint32_t)id;
            printf("write %u as %s \n", (uint32_t)id, str);
            label_mod.emplace(size, id);
            return 0;
        }
        else
        {
            return txt::setc(code, next, str);
        }
    }
    virtual void comment(const char* line, uint64_t bytes)
    {
        std::string label;
        if (find_label(line, label) == true)
        {
            //label_infos.emplace(label, bytes >> 1);
            auto id = label_id(label);

            label_info* info = (label_info*)malloc(sizeof(label_info) + src.length() + label.length() + 2);
            info->pos = bytes >> 1;
            info->line = lines;
            info->label_len = label.length();
            info->file_len = src.length();
            memcpy(info->data, label.c_str(), label.length());
            info->data[label.length()] = '\0';
            memcpy(info->data + label.length() + 1, src.c_str(), src.length());
            info->data[label.length() + 1 + src.length()] = '\0';

            label_infos.emplace(id, info);
        }
    }
    virtual int line(const code_t* code, int max_bytes, const char* orig)
    {
        int bytes = 2;
        switch (code->id)
        {
        case CODE_SETS:
            bytes += 2;
            break;
        case CODE_SETI:
            bytes += 4;
            break;
        case CODE_SETL:
            bytes += 8;
            break;
        case CODE_NOP:
            bytes += (code->ex << 1);
            break;
        default:
            break;
        }
        return write_code(code, bytes);
    }

    virtual int write_code(const code_t* code, int bytes)     = 0;
};

class compile_2_file : public compile
{
public:
    //compile_2_file(const char* file) : fd(open(file, O_WRONLY | O_CREAT | O_TRUNC))   {}
    compile_2_file(const char* file) :
        fd(open("./", O_TMPFILE | O_RDWR, 0666)),
        out(file)
    {}

    int             fd;
    std::string     out;
    std::unordered_map<uint32_t, std::string>   string_ids;

    virtual uint32_t setc(code_t& code, intptr_t* next, const char* str)
    {
        auto i = compile::setc(code, next, str);
        if (i > 0)
        {
            string_ids.emplace(i, str);
            *next = (i << 1) + 1;
        }
        return i;
    }

    virtual int write_code(const code_t* code, int bytes)
    {
        return write(fd, code, bytes);
    }

    virtual ~compile_2_file()
    {
        int real = open(out.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (real >= 0)
        {
            uint64_t end = 0;

            for (auto& it : string_ids)
            {
                uint32_t id = (it.first << 1) + 1;
                write(real, &id, sizeof(uint32_t));
                write(real, it.second.c_str(), it.second.length() + 1);
                printf("remap %u - %u - %s\n", it.first, id, it.second.c_str());
            }
            write(real, &end, sizeof(uint64_t));

            std::map<int64_t, uint32_t> offsets;
            off64_t last = 0;
            for (auto& it : label_infos)
            {
                offsets.emplace(it.first, last);
                auto s = write(real, it.second, sizeof(label_info) + it.second->label_len + it.second->file_len + 2);
                last += s;
                delete it.second;
            }
            write(real, &end, sizeof(uint64_t));
            
            lseek(fd, 0, SEEK_SET);
            last = 0;
            for (auto& it : label_mod)
            {
                copy_file_range(fd, &last, real, NULL, it.first - last, 0);
                lseek(fd, last, SEEK_SET);
                auto o = offsets.find(it.second);
#pragma pack(1)
                struct
                {
                    code_t      code;
                    uint32_t    label;
                } sl;
#pragma pack()
                read(fd, &sl, sizeof(sl));
                if ((sl.code.id != CODE_SETI) || (sl.code.ex != 0x09) || (sl.label != it.second))
                {
                    printf("%u\n", sl.label);
                    assert(0);
                }
                sl.label = o->second;
                write(real, &sl, sizeof(sl));
                last += sizeof(sl);
            }
            //int r = copy_file_range(fd, NULL, real, NULL, size, 0);
            //printf("%d : %d - %d : %d - %s\n", r, fd, real, errno, strerror(errno));

            close(real);
        }
        close(fd);
    }
};

class compile_2_run : public compile
{
public:
    std::vector<code_t>     codes;

    virtual ~compile_2_run()
    {
        if (codes.size() > 0)
        {
            int64_t exit = 0;
            bool r = regvm_exec(vm, &codes.front(), codes.size(), &exit);
            int color = (r == true) ? 32 : 31;
            printf("\n\n\e[%dm run result %d : %lld\e[0m\n\n", color, r, (long long)exit);
        }
    }

    virtual int write_code(const code_t* code, int bytes)
    {
        const int count = bytes >> 1;
        for (int i = 0; i < count; i++)
        {
            codes.emplace_back(code[i]);
        }
        return bytes;
    }
};

class bin : public run
{
public:
    int fd              = -1;
    int size            = -1;
    char* data          = NULL;
    code_t* start       = NULL;
    std::unordered_map<uint32_t, std::string>   strtab;

    virtual ~bin()
    {
        if (data != NULL) munmap((void*)data, size);
        if (fd >= 0) close(fd);
    }

    virtual bool prepare(const char* file)
    {
        run::prepare(file);

        fd = open(file, O_RDONLY);
        struct stat st;
        fstat(fd, &st);
        data = (char*)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        uint32_t* id = (uint32_t*)data;
        while (*id != 0)
        {
            const char* s = (const char*)(&id[1]);
            printf("reload %u - %s\n", *id, s);
            strtab.emplace(*id, s);
            id = (uint32_t*)(s + strlen(s) + 1);
        }
        start = (code_t*)&id[2];
        size = st.st_size - (((char*)start) - data);
        regvm_irq_set(vm, IRQ_STR_RELOCATE, reloate_str, this);

        return fd >= 0;
    }
    virtual bool go(void)
    {
        int64_t exit = 0;
        bool r = regvm_exec(vm, start, size / sizeof(code_t), &exit);
        int color = (r == true) ? 32 : 31;
        printf("\n\n\e[%dm run result %d : %lld\e[0m\n\n", color, r, (long long)exit);
        return true;
    }
    static int64_t reloate_str(struct regvm* vm, void* arg, int irq, code_t code, int offset, void* extra)
    {
        intptr_t id = (intptr_t)extra;
        bin* self = (bin*)arg;
        auto it = self->strtab.find(id);
        return (it != self->strtab.end()) ? (intptr_t)it->second.c_str() : 0;
    }
};

const char* HELP = R"(tester for regvm
USAGE tester [-f file] [-r] [-v] [-c out]
options:
    -f {file}       input file
    -r              run code file
    -s              run code file line by line
    -b              run binrary code file
    -c {out}        compile to binary code
    -v              verbose mode
)";

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        printf("Need input code file\n");
        return 0;
    }

    run* o = NULL;
    const char* file = NULL;

    const char* opts = "f:c:rsbhv";
    int opt = 0;
    while ((opt = getopt(argc, argv, opts)) != -1)
    {
        switch (opt)
        {
        case 'r':
            o = new compile_2_run();
            break;
        case 's':
            o = new step();
            break;
        case 'c':
            o = new compile_2_file(optarg);
            break;
        case 'b':
            o = new bin();
            break;
        case 'f':
            file = optarg;
            break;
        case 'v':
            verbose += 1;
            break;
        case 'h':
            printf("%s\n", HELP);
            return 0;
        }
    }

    if (o == NULL) return 0;

    o->prepare(file);

    o->go();

    delete o;
    //printf("\n\n%d bytes total\n", r);

    return 0;
}
