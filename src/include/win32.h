#pragma once

#if defined(_MSC_VER)

#include "getopt.h"

#if defined(__clang__)
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#endif

//#include <string.h>
//#include <intrin.h>

//#define memrchr _memrchr

#endif 
