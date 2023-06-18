/**
*  Copyright (C) Manas Kamal Choudhury 2021
*
*  /PROJECT - Aurora Xeneva v1.0
*  /AUTHOR  - Manas Kamal Choudhury
*
* ==============================================
*/

#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <_xeneva.h>

#ifdef __cplusplus
XE_EXTERN{
#endif

#include <stdarg.h>


#define max(a, b)  ((a) > (b) ? (a) : (b))
#define min(a, b)  ((a) < (b) ? (a) : (b))
#define offsetof(type, field)  ((unsigned long) &(((type *)0L)->field))

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define RAND_MAX  (0x7fffffff)

	XE_EXPORT int atoi(const char*);
	XE_EXPORT long atol(const char*);
	XE_EXPORT long atoll(const char *);
	XE_EXPORT double atof(const char*);
	XE_EXPORT int abs(int);
	XE_EXPORT void* malloc(unsigned int);
	XE_EXPORT void free(void* ptr);
	XE_EXPORT void* realloc(void* address, unsigned int new_size);
	XE_EXPORT void* calloc(unsigned long long num, unsigned long long size);
	XE_EXPORT int mblen(const char*, size_t);
	XE_EXPORT size_t mbstowcs(wchar_t *, const char*, size_t);
	XE_EXPORT int mbtowc(wchar_t*, const char*, size_t);
	XE_EXPORT char* mkdtemp(char*);
	XE_EXPORT char* mkstemp(char *);
	XE_EXPORT int rand(void);
#define random() rand()
	XE_EXPORT size_t wcstombs(char*, const wchar_t *, size_t);
	XE_EXPORT int wctomb(char*, wchar_t);
	XE_EXPORT void qsort(void* base, size_t num, size_t size, int(*comparator)(const void*, const void*));

	XE_EXPORT char* sztoa(size_t value, char* str, int base);
	XE_EXPORT char* getenv(const char*);
	XE_EXPORT void exit(int errno);

	XE_EXPORT int vsprintf(char *str, const char *format, va_list ap);
	XE_EXPORT long strtol(const char* nptr, char** endptr, int base);
	XE_EXPORT unsigned long strtoul(const char* nptr, char** endptr, int base);
	XE_EXPORT void itoa_s(int i, unsigned base, char* buf);
	XE_EXPORT void PrintOSName();

#ifdef __cplusplus
}
#endif


#endif