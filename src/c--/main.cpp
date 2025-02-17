#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <map>
#include <list>
#include <vector>
#include <string>
#include <iterator>
#include <string_view>
#include <unordered_map>

#include "inst.h"
#include "common.h"
#include "parser.h"

#include <log.h>
#include <code.h>
#include <regvm.h>


struct var
{
    DATA_TYPE           type;
    uv                  init;
    std::string_view    name;
};

//struct func
//{
//    DATA_TYPE           ret;
//    std::vector<var>    args;
//};
//
//std::unordered_map<std::string_view, func> funcs;



[[maybe_unused]]
static char t1[] = R"(
//int a;
//double b = 12.34;
//int c = -64898;
//double d = b;
//int e = 2 + 6 - 5;
//int f = 2 + (6 - 5) + 9;
//int g = 1 + 2 * 3;
//int h = 10 * 20 - 30;
int i = 6 + 10 * (87 + 51) - 30;
int j = echo(i, 10 * 20 - 30, -18.484);
)";


[[maybe_unused]]
static char t2[] = R"(
#include <stdio.h>
int main()
{
    int a;
    a = 1 + 2;
    printf("%d\n", a);
    return 0;
}
)";

const char* HELP = R"(tester for vcc
USAGE tester [file] [-r]
options:
    [file]          source file
    -r              run code file
    -p              print inst only
    -o              output to file
)";

bool op_run(const std::string_view& name, const func* f, insts_t* insts)
{
    auto op = &inst::print_bin;
    char* codes = NULL;
    size_t bytes = 0;
    FILE* fp = open_memstream(&codes, &bytes);

    if (insts == NULL)
    {
        f->print(op, fp);
    }
    else
    {
        for (auto& it : *insts)
        {
            (it->*op)(fp);
        }
    }

    fclose(fp);

    if LOG_ENBALE_D
    {
        LOGI("func : %s ...", VIEW(name));
        const int line = 16;
        int j = 0;
        const unsigned char* p = (const unsigned char*)codes;
        for (int i = 0; i < (int)bytes; i++)
        {
            if (j++ >= line)
            {
                printf("\n");
                j = 1;
            }
            printf("%02X ", p[i]);
        }
        printf("\n\n");
    }


    extern int mem_init(void);
    static auto vm = regvm_init(1, mem_init);
    if (insts != NULL)
    {

        int64_t exit = 0;
        bool r = regvm_exec(vm, (code_t*)codes, bytes >> 2, &exit);
        if ((r == true) && (f != NULL))
        {
            //TODO
            //run the main function ?
            //such as :
            //r = regvm_func_exec(vm, f->id, &exit);
        }

        regvm_exit(vm);
        vm = NULL;

        LOGI("run : %d\n", r);
    }
    else
    {
        regvm_func(vm, f->id, (code_t*)codes, bytes >> 2, NULL, VM_CODE_COPY);
    }

    free(codes);

    return true;
}

bool op_print(const std::string_view& name, const func* f, insts_t* insts)
{
    LOGI("func : %s ...", VIEW(name));
    auto op = &inst::print;

    if (insts == NULL)
    {
        f->print(op, stdout);
    }
    else
    {
        for (auto& it : *insts)
        {
            (it->*op)(stdout);
        }
    }

    printf("\n");
    return true;
}

bool op_asm(const std::string_view& name, const func* f, insts_t* insts)
{
    LOGI("func : %s ...", VIEW(name));
    auto op = &inst::print_asm;

    if (insts == NULL)
    {
        f->print(op, stdout);
    }
    else
    {
        for (auto& it : *insts)
        {
            (it->*op)(stdout);
        }
    }

    printf("\n");
    return true;
}

int main(int argc, char** argv)
{
    insts_t insts;

    const char* src = t1;
    int fd = -1;
    struct stat st;
    if (argc > 1)
    {
        fd = ::open(argv[1], O_RDONLY);
        fstat(fd, &st);
        src = (const char*)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    }

    parser par;
    auto r = par.go(argv[1], src, insts);
    LOGD("parse : %d", r);
    if (r == false)
    {
        return 1;
    }


    const char* opts = "rps";
    int opt = 0;
    bool (*op)(const std::string_view&, const func*, insts_t*) = NULL;
    while ((opt = getopt(argc - 1, argv + 1, opts)) != -1)
    {
        switch (opt)
        {
        case 'r':
            op = op_run;
            break;
        case 'p':
            op = op_print;
            break;
        case 's':
            op = op_asm;
            break;
        default:
            break;
        }
    }

    for (auto& it : par.funcs)
    {
        op(it.first, &it.second, NULL);
    }
    auto it = par.funcs.find("main");
    op(".crt.entry", (it == par.funcs.end()) ? NULL : &it->second, &insts);

    for (auto& it : insts)
    {
        delete it;
    }

    if (fd >= 0)
    {
        munmap((void*)src, st.st_size);
        close(fd);
    }

    return 0;
}

