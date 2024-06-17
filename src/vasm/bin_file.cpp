#include "bin_file.h"

#include <fcntl.h>

using namespace vasm;

bin_file::pass2::pass2(bin_file& o) : strids::pass2(o), data(o)
{
    fd = ::open(o.out.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
