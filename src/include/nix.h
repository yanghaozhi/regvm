#pragma once

#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>


#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)


template <typename F> bool map_file(const char* file, F func)
{
    int fd = open(file, O_RDONLY);
    struct stat st;
    fstat(fd, &st);
    auto d = (char*)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (func(d) == false)
    {
        LOGE("parse file : %s ERROR !!!", file);
        return false;
    }

    munmap((void*)d, st.st_size);
    close(fd);

	return true;
}

