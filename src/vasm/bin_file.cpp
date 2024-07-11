#include "bin_file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <regvm.h>
#include <debug.h>
#include <irq.h>

#include <regvm_ext.h>


using namespace vasm;

bool bin_file::open(const char* name)
{
    if (bin != name)
    {
        return strids::open(name);
    }

    int fd = ::open(bin.c_str(), O_RDONLY);
    struct stat st;
    fstat(fd, &st);
    auto data = (char*)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    uint32_t* id = (uint32_t*)data;
    while (*id != 0)
    {
        const char* s = (const char*)(&id[1]);
        printf("reload %u - %s\n", *id, s);
        strs.emplace(*id, s);
        id = (uint32_t*)(s + strlen(s) + 1);
    }
    code_t* start = (code_t*)&id[2];
    int size = st.st_size - (((char*)start) - data);

    auto vm = regvm_init();

    //regvm_irq_set(vm, IRQ_TRAP, debug_trap, NULL);
    regvm_irq_set(vm, IRQ_STR_RELOCATE, str_relocate, this);

    int64_t exit = 0;
    bool r = regvm_exec(vm, start, size >> 1, &exit);
    LOGI("run result : %d and exit code : %lld", r, (long long)exit);

    regvm_exit(vm);



    //do NOT run next step !!!
    return false;
}

bool bin_file::finish()
{
    return strids::finish();
}

bin_file::pass2::pass2(bin_file& o) : strids::pass2(o), data(o)
{
    fd = ::open(o.bin.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
}

bin_file::pass2::~pass2()
{
    close(fd);
}

void bin_file::pass2::before()
{
    strids::pass2::before();
}

void bin_file::pass2::after()
{
    strids::pass2::after();
}

int bin_file::pass2::write_data(const void* data, int bytes)
{
    return write(fd, data, bytes);
}

int bin_file::pass2::write_code(const code_t* code, int bytes)
{
    return write(fd, code, bytes);
}

int64_t bin_file::str_relocate(regvm* vm, void* arg, code_t code, int offset, void* extra)
{
    bin_file* f = (bin_file*)arg;
    intptr_t k = (intptr_t)extra;
    auto it = f->strs.find(k);
    const char* v = (it != f->strs.end()) ? it->second : NULL;
    return (int64_t)v;
}
