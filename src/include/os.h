#pragma once

#include <string.h>

#if defined(_MSC_VER)

#include <string.h>
#include <intrin.h>

//#define memrchr _memrchr

#endif 



#if defined(__GNUC__) || defined(__clang__)

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#endif

