#include "mem_run.h"

#include <regvm.h>
#include "../ext/regext.h"

using namespace vasm;

mem_2_run::mem_2_run() : buf(NULL)
{
}

mem_2_run::~mem_2_run()
{
    free(buf);
}

bool mem_2_run::finish()
{
    const int line = 16;
    int j = 0;
    const unsigned char* p = (const unsigned char*)buf;
    INFO("total {} bytes of codes", code_bytes);
    for (int i = 0; i < code_bytes; i++)
    {
        if (j++ >= line)
        {
            printf("\n");
            j = 1;
        }
        printf("%02X ", p[i]);
    }
    printf("\n\n");

    auto vm = regvm_init(&var_ext);

    int64_t exit = 0;
    bool r = regvm_exec(vm, codes, code_bytes >> 1, &exit);
    INFO("run result : {} and exit code : {}", r, exit);

    regvm_exit(vm);
    return r;
}

mem_2_run::pass1::pass1(mem_2_run& o) : compile::pass1(o), data(o)
{
}

int mem_2_run::pass1::write_code(const code_t* code, int bytes)
{
    data.code_bytes += bytes;
    return bytes;
}

mem_2_run::pass2::pass2(mem_2_run& o) : compile::pass2(o), data(o)
{
    data.buf = malloc(data.code_bytes);
    cur = data.codes;
}

int mem_2_run::pass2::write_code(const code_t* code, int bytes)
{
    memcpy(cur, code, bytes);
    cur += (bytes >> 1);
    return bytes;
}

