/**
* @file timer.h
*
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

#ifndef __TIMER_H__
#define __TIMER_H__

#include <aurora.h>

typedef void (*AuroraTimerCallback)(void* param);

typedef struct _kernel_timer_ {
	uint64_t expireUS;
	uint64_t intervalUS;
	AuroraTimerCallback handler;
	void* param;
	uint8_t active;
}AuKernelTimer;

#define AURORA_MAX_TIMER  32


/**
 * @brief AuroraTimerInitialize -- initialize aurora
 * timer
 */
extern void AuroraTimerInitialize();

/**
 * @brief AuroraTimerStart -- start kernel timer in one shot mode
 * @param delay_ms -- at what millisecond it should fire from now
 * @param handler -- handler function
 * @param param -- extra param data to pass to handler
 */
AU_EXTERN AU_EXPORT int AuroraTimerStart(uint32_t delay_ms, AuroraTimerCallback handler, void* param, int idx);

/**
 * @brief AuroraTimerStartPeriodic -- start a periodic timer
 * @param interval_ms -- interval between one shot to another shot
 * @param handler -- handler function
 * @param param -- extra data to pass to handler
 */
AU_EXTERN AU_EXPORT int AuroraTimerStartPeriodic(uint32_t interval_ms, AuroraTimerCallback handler, void* param);


/**
 * @brief AuroraTimerIsActive -- check if timer is active
 * @param tNum -- timer index
 */
AU_EXTERN AU_EXPORT bool AuroraTimerIsActive(int tNum);
/**
 * @brief AuroraTimerCancel -- cancel a timer
 * @param tNum -- timer number
 */
AU_EXTERN AU_EXPORT void AuroraTimerCancel(int tNum);

/**
 * @brief AuroraTimerTick -- called by kernel
 * to handle all expired timers
 */
void AuroraTimerTick();


AU_EXTERN AU_EXPORT uint64_t AuGetCurrentMS();

AU_EXTERN AU_EXPORT uint64_t AuGetCurrentUS();


#endif
