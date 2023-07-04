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

	XE_LIB int atoi(const char*);
	XE_LIB long atol(const char*);
	XE_LIB long atoll(const char *);
	XE_LIB double atof(const char*);
	XE_LIB int abs(int);
	XE_LIB void* malloc(unsigned int);
	XE_LIB void free(void* ptr);
	XE_LIB void* realloc(void* address, unsigned int new_size);
	XE_LIB void* calloc(unsigned long long num, unsigned long long size);
	XE_LIB int mblen(const char*, size_t);
	XE_LIB size_t mbstowcs(wchar_t *, const char*, size_t);
	XE_LIB int mbtowc(wchar_t*, const char*, size_t);
	XE_LIB char* mkdtemp(char*);
	XE_LIB char* mkstemp(char *);
	XE_LIB int rand(void);
#define random() rand()
	XE_LIB size_t wcstombs(char*, const wchar_t *, size_t);
	XE_LIB int wctomb(char*, wchar_t);
	XE_LIB void qsort(void* base, size_t num, size_t size, int(*comparator)(const void*, const void*));

	XE_LIB char* sztoa(size_t value, char* str, int base);
	XE_LIB char* getenv(const char*);
	XE_LIB void exit(int errno);

	XE_LIB int vsprintf(char *str, const char *format, va_list ap);
	XE_LIB long strtol(const char* nptr, char** endptr, int base);
	XE_LIB unsigned long strtoul(const char* nptr, char** endptr, int base);
	XE_LIB void itoa_s(int i, unsigned base, char* buf);
	XE_LIB void PrintOSName();

#ifdef __cplusplus
}
#endif


#endif