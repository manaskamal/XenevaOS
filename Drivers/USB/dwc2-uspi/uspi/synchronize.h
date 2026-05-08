//
// synchronize.h
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
#ifndef _uspi_synchronize_h
#define _uspi_synchronize_h

#include "uspi/macros.h"
#include "uspi/types.h"
#include <Hal/AA64/aa64lowlevel.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// Interrupt synchronization
//
void uspi_EnterCritical (void);		// disable interrupts (nested calls possible)
void uspi_LeaveCritical (void);		// enable interrupts (nested calls possible)

#ifndef AARCH64

#if RASPPI == 1

//
// Cache control
//
#define InvalidateInstructionCache()	\
				__asm volatile ("mcr p15, 0, %0, c7, c5,  0" : : "r" (0) : "memory")
#define FlushPrefetchBuffer()	__asm volatile ("mcr p15, 0, %0, c7, c5,  4" : : "r" (0) : "memory")
#define FlushBranchTargetCache()	\
				__asm volatile ("mcr p15, 0, %0, c7, c5,  6" : : "r" (0) : "memory")
#define InvalidateDataCache()	__asm volatile ("mcr p15, 0, %0, c7, c6,  0" : : "r" (0) : "memory")
#define CleanDataCache()	__asm volatile ("mcr p15, 0, %0, c7, c10, 0" : : "r" (0) : "memory")

void uspi_CleanAndInvalidateDataCacheRange (u32 nAddress, u32 nLength) MAXOPT;

//
// Barriers
//
#define DataSyncBarrier()	__asm volatile ("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")
#define DataMemBarrier() 	__asm volatile ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")

#define InstructionSyncBarrier() FlushPrefetchBuffer()
#define InstructionMemBarrier()	FlushPrefetchBuffer()

#else	// #if RASPPI == 1

//
// Cache control
//
#define InvalidateInstructionCache()	//\
				//__asm volatile ("mcr p15, 0, %0, c7, c5,  0" : : "r" (0) : "memory")
#define FlushPrefetchBuffer()	isb_flush() //__asm volatile ("isb" ::: "memory")
#define FlushBranchTargetCache()	//\
				//__asm volatile ("mcr p15, 0, %0, c7, c5,  6" : : "r" (0) : "memory")

void uspi_CleanAndInvalidateDataCacheRange(u32 nAddress, u32 nLength); // MAXOPT;

//
// Barriers
//
#define DataSyncBarrier()   dsb_sy_barrier()//	 __asm volatile ("dsb" ::: "memory")
#define DataMemBarrier() 	dmb_sy() //__asm volatile ("dmb" ::: "memory")

#define InstructionSyncBarrier() isb_flush(); // __asm volatile ("isb" ::: "memory")
#define InstructionMemBarrier()	 isb_flush();// __asm volatile ("isb" ::: "memory")

#endif	// #if RASPPI == 1

#else	// #ifdef AARCH64

//
// Cache control
//
void uspi_CleanAndInvalidateDataCacheRange (u64 nAddress, u64 nLength) MAXOPT;

//
// Barriers
//
#define DataSyncBarrier()	__asm volatile ("dsb sy" ::: "memory")
#define DataMemBarrier() 	__asm volatile ("dmb sy" ::: "memory")

#endif	// #ifdef AARCH64

#define CompilerBarrier()	__asm volatile ("" ::: "memory")

#ifdef __cplusplus
}
#endif

#endif
