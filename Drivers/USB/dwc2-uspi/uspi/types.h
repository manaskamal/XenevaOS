//
// types.h
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _uspi_types_h
#define _uspi_types_h

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
#ifndef AARCH64
typedef unsigned long long	u64;
#else
typedef unsigned long		u64;
#endif

typedef signed char		s8;
typedef signed short		s16;
typedef signed int		s32;
#ifndef AARCH64
typedef signed long long	s64;
#else
typedef signed long		s64;
#endif

#ifndef AARCH64
typedef s32			intptr;
typedef u32			uintptr;
#else
typedef s64			intptr;
typedef u64			uintptr;
#endif

typedef int		boolean;
#define FALSE		0
#define TRUE		1

//typedef unsigned long	size_t;
typedef long		ssize_t;

#endif
