#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "log.h"
#include "mem_run.h"
#include "bin_file.h"
#include "inst.h"

using namespace vasm;


const char* HELP = R"(tester for regvm
USAGE tester [file] [-r] [-v] [-c out]
options:
    -f {file}       input file
    -r              run code file
    -s              show run code
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

    const char* file = argv[1];

    parser* obj = NULL;
    void (inst::*op)(FILE*) const = &inst::print;
    FILE* fp = stdout;

    const char* opts = "c:rsbhv";
    int opt = 0;
    while ((opt = getopt(argc - 1, argv + 1, opts)) != -1)
    {
        switch (opt)
        {
        case 'r':
            obj = new mem_run();
            op = &inst::print_bin;
            break;
        case 's':
            obj = new mem_run();
            op = &inst::print_asm;
            break;
        case 'c':
            //op = new TOP<bin_file>(optarg);
            //o = new compile_2_file(optarg);
            break;
        case 'b':
            //op = new TOP<bin_file>(file);
            //o = new bin();
            break;
        case 'v':
            break;
        case 'h':
            printf("%s\n", HELP);
            return 0;
        }
    }

    if (obj == NULL)
    {
        LOGW("Does NOT find input file !!!");
        return 0;
    }


    int fd = open(file, O_RDONLY);
    struct stat st;
    fstat(fd, &st);
    auto d = (char*)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (obj->go(d) == false)
    {
        LOGE("parse file : %s ERROR !!!", file);
        return 1;
    }

    munmap((void*)d, st.st_size);
    close(fd);

    obj->finish(fp, op);

    delete obj;

    return 0;
}
