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
#include "statements.h"

#include <log.h>
#include <code.h>
#include <regvm.h>


struct var
{
    DATA_TYPE           type;
    uv                  init;
    std::string_view    name;
};

struct func
{
    DATA_TYPE           ret;
    std::vector<var>    args;
};

std::unordered_map<std::string_view, func> funcs;


bool grammar(std::vector<inst>& insts, const char* src)
{
    LOGT("%s", src);

    parser par;
    decl_var_only       dvo(&par);
    decl_var_init       dvi(&par);
    call_func_no_ret    cfnr(&par);
    assign_var          avar(&par);

    while ((src != NULL) && (*src != '\0'))
    {
        src = par.statement(src);
    }

    insts.swap(par.insts);
    return (src != NULL) ? true : false;
}



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


int main(int argc, char** argv)
{
    std::vector<inst> insts;

    const char* src = t1;
    int fd = -1;
    struct stat st;
    if (argc > 1)
    {
        fd = ::open(argv[1], O_RDONLY);
        fstat(fd, &st);
        src = (const char*)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    }

    auto r = grammar(insts, src);
    LOGD("parse : %d", r);
    if (r == false)
    {
        return 1;
    }


    void (inst::*op)(FILE*) = &inst::print_txt;
    const char* opts = "rps";
    int opt = 0;
    FILE* fp = stdout;
    char* codes = NULL;
    size_t bytes = 0;
    while ((opt = getopt(argc - 1, argv + 1, opts)) != -1)
    {
        switch (opt)
        {
        case 'r':
            op = &inst::print_bin;
            fp = open_memstream(&codes, &bytes);
            break;
        case 'p':
            op = &inst::print;
            break;
        case 's':
            op = &inst::print_txt;
            break;
        default:
            break;
        }
    }

    for (auto& it : insts)
    {
        (it.*op)(fp);
    }

    if (fp != stdout)
    {
        fclose(fp);
    }

    if (codes != NULL)
    {
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


        auto vm = regvm_init();

        int64_t exit = 0;
        bool r = regvm_exec(vm, (code_t*)codes, bytes >> 1, &exit);

        regvm_exit(vm);

        printf("run : %d\n", r);
    }
    //auto r = grammar(t2);
    //printf("grammar : %d\n", r);

    if (fd >= 0)
    {
        munmap((void*)src, st.st_size);
        close(fd);
    }

    return 0;
}

