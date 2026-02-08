
#ifndef __VA_LIST_H__
#define __VA_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

// Use Compiler Builtins for va_list type
#if defined(__GNUC__) || defined(__clang__)
    #ifndef _VALIST
    #define _VALIST
    typedef __builtin_va_list va_list;
    typedef __builtin_va_list _va_list_;
    #endif
#else
    #ifndef _VALIST
    #define _VALIST
        typedef unsigned char *_va_list_;
        typedef unsigned char * va_list;
    #endif

    #define STACKITEM int
    #define VA_SIZE(TYPE)   \
        ((sizeof(TYPE)+sizeof(STACKITEM)-1) \
        & ~(sizeof(STACKITEM)-1))

    #define va_start(AP, LASTARG) \
        (AP = ((_va_list_)&(LASTARG)+VA_SIZE(LASTARG)))

    #define va_end(AP)

    #define va_arg(AP, TYPE)  \
        (AP += VA_SIZE(TYPE), *((TYPE *)(AP - VA_SIZE(TYPE))))
#endif

#ifdef __cplusplus
}
#endif

#endif
