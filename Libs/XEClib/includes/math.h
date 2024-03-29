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

#ifndef __MATH_H__
#define __MATH_H__

#include <_xeneva.h>

#ifdef __cplusplus
XE_EXTERN{
#endif


#define M_E         2.7182818284590452354  // e
#define M_LOG2E     1.4426950408889634074  // log_2 e
#define M_LOG10E    0.43429448190325182765 // log_10 e
#define M_LN2       0.69314718055994530942 // log_e 2
#define M_LN10      2.30258509299404568402 // log_e 10
#define M_PI        3.14159265358979323846 // pi
#define M_PI_2      1.57079632679489661923 // pi/2
#define M_PI_4      0.78539816339744830962 // pi/4
#define M_1_PI      0.31830988618379067154 // 1/pi
#define M_2_PI      0.63661977236758134308 // 2/pi
#define M_2_SQRTPI  1.12837916709551257390 // 2/sqrt(pi)
#define M_SQRT2     1.41421356237309504880 // sqrt(2)
#define M_SQRT1_2   0.70710678118654752440 // 1/sqrt(2)

	XE_LIB double ceil(double);
	XE_LIB double cos(double);
	XE_LIB float cosf(float);
	XE_LIB double fabs(double);
	XE_LIB float fabsf(float);
	XE_LIB double floor(double);
	XE_LIB float floorf(float);
	XE_LIB double fmod(double, double);
	XE_LIB double modf(double, double *);
	XE_LIB double pow(double, double);
	XE_LIB double sin(double);
	XE_LIB float sinf(float);
	XE_LIB double sqrt(double);
	XE_LIB float sqrtf(float x);
	XE_LIB double tan(double);
	XE_LIB float tanf(float);
	XE_LIB double frexp(double x, int *exp);

#ifdef __cplusplus
}
#endif

#endif