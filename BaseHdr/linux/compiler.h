
#ifndef __COMPILER_H__
#define __COMPILER_H__

#if defined(_MSC_VER)
#define likely(x)  ((x) ? (1) : (__assume(0), 0))
#define unlikely(x) ((x) ? (__assume(0), 1) : 0)
#elif defined(__GNUC__) || defined(__clang__)
#define likely(x)  __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x)  (x)
#define unlikely(x)  (x)
#endif

#endif