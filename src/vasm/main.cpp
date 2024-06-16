#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "mem_run.h"

using namespace vasm;

struct OP
{
    virtual ~OP()   {}
    virtual bool go(const char* name)   = 0;
};


template <typename T> struct TOP : public OP
{
    virtual bool go(const char* name)
    {
        T op;
        if (op.open(name) == false)
        {
            return false;
        }

        typename T::pass1 s1(op);
        if (s1.scan() == false)
        {
            return false;
        }

        typename T::pass2 s2(op);
        if (s2.scan() == false)
        {
            return false;
        }

        if (op.finish() == false)
        {
            return false;
        }

        return true;
    }
};

const char* HELP = R"(tester for regvm
USAGE tester [file] [-r] [-v] [-c out]
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

    const char* file = argv[1];

    OP* op = NULL;
    const char* opts = "c:rsbhv";
    int opt = 0;
    while ((opt = getopt(argc - 1, argv + 1, opts)) != -1)
    {
        switch (opt)
        {
        case 'r':
            op = new TOP<mem_2_run>();
            break;
        case 's':
            //o = new step();
            break;
        case 'c':
            //o = new compile_2_file(optarg);
            break;
        case 'b':
            //o = new bin();
            break;
        case 'v':
            logging::set_level(0);
            break;
        case 'h':
            printf("%s\n", HELP);
            return 0;
        }
    }

    if (op == NULL) return 0;

    op->go(file);

    delete op;

    return 0;
}
