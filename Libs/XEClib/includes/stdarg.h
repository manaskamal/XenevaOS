/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**/

#ifndef __STDARG_H__
#define __STDARG_H__


#ifdef __cplusplus
extern "C"
{
#endif


	/* va list parameter list */
#ifdef ARCH_RISCV64
	typedef __builtin_va_list va_list;
#else
	typedef unsigned char *va_list;
#endif


	/* width of stack == width of int */
#define	STACKITEM	int64_t

	/* round up width of objects pushed on stack. The expression before the
	& ensures that we get 0 for objects of size 0. */
#define	VA_SIZE(TYPE)					\
	((sizeof(TYPE)+sizeof(STACKITEM)-1)	\
	& ~(sizeof(STACKITEM)-1))

#ifdef ARCH_X64
	/* &(LASTARG) points to the LEFTMOST argument of the function call
	(before the ...) */
#define	va_start(AP, LASTARG)	\
	(AP = ((va_list)&(LASTARG)+VA_SIZE(LASTARG)))

	/* nothing for va_end */
#define va_end(AP)

#define va_arg(AP, TYPE)	\
	(AP += VA_SIZE(TYPE), *((TYPE *)(AP - VA_SIZE(TYPE))))
#elif ARCH_ARM64
#define va_start(ap,last) \
     ((ap) = (va_list)(&(last)) + 8)

#define va_arg(ap,T) \
     (*(T*)((ap) += 8, (ap) - 8))

#define va_end(ap) \
     ((ap) = (va_list)0)

#elif ARCH_RISCV64
typedef __builtin_va_list __gnuc_va_list;
#define va_start(v, l)	__builtin_va_start(v, l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v, l)	__builtin_va_arg(v, l)
#endif

#ifdef __cplusplus
}
#endif


#endif