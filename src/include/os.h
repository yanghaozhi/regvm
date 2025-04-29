#pragma once

#if defined(_MSC_VER) && !defined(__clang__)

#define likely(x) (x)
#define unlikely(x) (x)

#define strdup _strdup

#endif // defined(_MSC_VER) && !defined(__clang__)

#if defined(__GNUC__) || defined(__clang__)

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#endif

