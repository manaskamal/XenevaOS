
#ifndef __STDARG_H__
#define __STDARG_H__
#include <va_list.h>

#ifdef __cplusplus
extern "C" {
#endif

// Use Compiler Builtins for robust cross-architecture support (x64, ARM64, RISC-V)
#if defined(__GNUC__) || defined(__clang__)
    typedef __builtin_va_list va_list;
    #define va_start(ap, last) __builtin_va_start(ap, last)
    #define va_arg(ap, type) __builtin_va_arg(ap, type)
    #define va_end(ap) __builtin_va_end(ap)
    #define va_copy(dest, src) __builtin_va_copy(dest, src)
#else
    // Fallback for older MSVC or purely stack-based archs (Legacy)
    #define	STACKITEM	int64_t
    #define	VA_SIZE(TYPE) ((sizeof(TYPE)+sizeof(STACKITEM)-1) & ~(sizeof(STACKITEM)-1))
    #define	va_start(AP, LASTARG) (AP = ((va_list)&(LASTARG)+VA_SIZE(LASTARG)))
    #define va_end(AP)
    #define va_arg(AP, TYPE) (AP += VA_SIZE(TYPE), *((TYPE *)(AP - VA_SIZE(TYPE))))
#endif

#ifdef __cplusplus
}
#endif

#endif
