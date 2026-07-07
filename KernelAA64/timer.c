/**
* @file timer.c
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

#include <timer.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <string.h>
#include <aucon.h>
#include <Drivers/uart.h>
#include <_null.h>
#include <signal.h>

static AuKernelTimer _timers[AURORA_MAX_TIMER];
static uint64_t _system_current_us;
static uint64_t _system_current_ms;

static inline uint64_t _get_current_us() {
	uint64_t cnt = get_cntpct_el0();
	uint64_t freq = get_cntfrq_el0();
	return (cnt * 1000000ULL) / freq;
}

static inline uint64_t _get_current_ms() {
	return _get_current_us() / 1000;
}

/**
 * @brief AuroraTimerInitialize -- initialize aurora
 * timer 
 */
void AuroraTimerInitialize() {
	for (int i = 0; i < AURORA_MAX_TIMER; i++) 
		memset(&_timers[i], 0, sizeof(AuKernelTimer));
	_system_current_us = 0;
	_system_current_ms = 0;
}

/**
 * @brief AuroraTimerStart -- start kernel timer in one shot mode
 * @param delay_ms -- at what millisecond it should fire from now
 * @param handler -- handler function
 * @param param -- extra param data to pass to handler
 */
int AuroraTimerStart(uint32_t delay_ms, AuroraTimerCallback handler, void* param, int idx) {
	if (idx != -1) {
		if (_timers[idx].active)
			return -1;
		_timers[idx].expireUS = _get_current_us() + (delay_ms * 1000ULL);
		_timers[idx].intervalUS = 0;
		_timers[idx].handler = handler;
		_timers[idx].param = param;
		_timers[idx].active = 1;
		return idx;
	}
	else {
		for (int i = 0; i < AURORA_MAX_TIMER; i++) {
			if (_timers[i].active) continue;

			_timers[i].expireUS = _get_current_us() + (delay_ms * 1000ULL);
			_timers[i].intervalUS = 0;
			_timers[i].handler = handler;
			_timers[i].param = param;
			_timers[i].active = 1;
			return i;
		}
	}
	return -1;
}

/**
 * @brief AuroraTimerStartPeriodic -- start a periodic timer
 * @param interval_ms -- interval between one shot to another shot
 * @param handler -- handler function
 * @param param -- extra data to pass to handler
 */
int AuroraTimerStartPeriodic(uint32_t interval_ms, AuroraTimerCallback handler, void* param) {
	for (int i = 0; i < AURORA_MAX_TIMER; i++) {
		if (_timers[i].active) continue;
		
		_timers[i].expireUS = _get_current_us() + (interval_ms * 1000ULL);
		_timers[i].intervalUS = interval_ms * 1000ULL;
		_timers[i].handler = handler;
		_timers[i].param = param;
		_timers[i].active = 1;

		return i;
	}
	return -1;
}

/**
 * @brief AuroraTimerCancel -- cancel a timer
 * @param tNum -- timer number
 */
void AuroraTimerCancel(int tNum) {
	if (tNum >= 0 && tNum < AURORA_MAX_TIMER) {
		_timers[tNum].active = 0;
	}
}

/**
 * @brief AuroraTimerIsActive -- check if timer is active
 * @param tNum -- timer index
 */
bool AuroraTimerIsActive(int tNum) {
	if (tNum >= 0 && tNum < AURORA_MAX_TIMER) {
		return _timers[tNum].active;
	}
	return 0;
}

/**
 * @brief AuroraTimerTick -- called by kernel
 * to handle all expired timers
 */
void AuroraTimerTick() {
	uint64_t now = _get_current_us();
	_system_current_us = now;
	_system_current_ms = _get_current_ms();
	
	for (int i = 0; i < AURORA_MAX_TIMER; i++) {
		if (!_timers[i].active) continue;
		if (now < _timers[i].expireUS)  continue;
		AuroraTimerCallback handler = _timers[i].handler;
		void* param = _timers[i].param;
	//	UARTDebugOut("Timer fired : %d \r\n", i);
		if (_timers[i].intervalUS > 0) {
			_timers[i].expireUS = now + _timers[i].intervalUS;
		}
		else
			_timers[i].active = 0;

		if (handler)
			handler(param);
	}
}

uint64_t AuGetCurrentMS() {
	return _system_current_ms;
}

uint64_t AuGetCurrentUS() {
	return _system_current_us;
}

void __kernel_timer_alarm_callback(void* param) {
	AA64Thread* thr = (AA64Thread*)param;
	AuAllocSignal(thr, SIGALRM);
}

/**
 * @brief AuTimerCalculateAlarm -- schedule a timer to one-shot mode
 * respective to given seconds
 * @param thr -- pointer to thread which requires one-shot timer
 * @param seconds -- amount of second to wait
 */
int AuTimerCalculateAlarm(AA64Thread* thr, uint64_t seconds) {
	uint64_t now = _get_current_us();
	int remaining_sec = 0;

	int existing_idx = -1;
	for (int i = 0; i < AURORA_MAX_TIMER; i++) {
		if (_timers[i].active && _timers[i].owner == thr &&
			_timers[i].handler == __kernel_timer_alarm_callback) {
			existing_idx = i;
			break;
		}
	}

	if(existing_idx != -1) {
		if (_timers[existing_idx].expireUS > now) {
			uint64_t remaining_us = _timers[existing_idx].expireUS - now;
			remaining_sec = (remaining_us + 999999) / 1000000;
		}
		_timers[existing_idx].active = 0;
		_timers[existing_idx].owner = NULL;
	}

	if (seconds == 0)
		return remaining_sec;

	int free_idx = -1;
	for (int i = 0; i < AURORA_MAX_TIMER; i++) {
		if (!_timers[i].active) {
			free_idx = i;
			break;
		}
	}

	if (free_idx == -1) 
		return remaining_sec;
	
	_timers[free_idx].expireUS = now + (seconds * 1000000ULL);
	_timers[free_idx].intervalUS = 0;
	_timers[free_idx].handler = __kernel_timer_alarm_callback;
	_timers[free_idx].param = (void*)thr;
	_timers[free_idx].owner = thr;
	_timers[free_idx].active = 1;

	return remaining_sec;
}


static inline uint64_t _timeval_to_us(const timeval_t* tv) {
	return (uint64_t)tv->tv_sec * 1000000ULL;
}

static inline uint64_t _us_to_timeval(uint64_t us, timeval_t* tv) {
	tv->tv_sec = us / 1000000ULL;
	tv->tv_usec = us % 1000000ULL;
}

#define ITIMER_REAL    0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF    2

/**
 *@brief AuTimerSetITimer -- posix standard implementation of 
 * setting periodic timer
 */
int AuTimerSetITimer(AA64Thread* thr, int which, const itimerval_t* newval, itimerval_t* oldval) {
	if (which != ITIMER_REAL)
		return -1;

	uint64_t now = _get_current_us();

	int idx = -1;
	for (int i = 0; i < AURORA_MAX_TIMER; i++) {
		if (_timers[i].active && _timers[i].owner == thr &&
			_timers[i].handler == __kernel_timer_alarm_callback) {
			idx = i;
			break;
		}
	}

	if (oldval) {
		if (idx != -1 && _timers[idx].expireUS > now) {
			_us_to_timeval(_timers[idx].expireUS - now, &oldval->it_value);
		}
		else {
			oldval->it_value.tv_sec = 0;
			oldval->it_value.tv_usec = 0;
		}
		_us_to_timeval(_timers[idx != -1 ? idx : 0].intervalUS, &oldval->it_interval);
		if (idx == -1) {
			oldval->it_interval.tv_sec = 0;
			oldval->it_interval.tv_usec = 0;
		}
	}

	uint64_t new_value_us = newval ? _timeval_to_us(&newval->it_value) : 0;
	uint64_t new_interval_us = newval ? _timeval_to_us(&newval->it_interval) : 0;

	if (new_value_us == 0) {
		if (idx != -1) {
			_timers[idx].active = 0;
			_timers[idx].owner = NULL;
		}
		return 0;
	}

	if (idx == -1) {
		for (int i = 0; i < AURORA_MAX_TIMER; i++) {
			if (!_timers[i].active) {
				idx = i;
				break;
			}
		}
		if (idx == -1)
			return -1;
	}
	UARTDebugOut("Seting kernel timer : %d \r\n", idx);
	_timers[idx].expireUS = now + new_value_us;
	_timers[idx].intervalUS = new_interval_us;
	_timers[idx].handler = __kernel_timer_alarm_callback;
	_timers[idx].param = (void*)thr;
	_timers[idx].owner = thr;
	_timers[idx].active = 1;

	return 0;
}

int AuTimerGetITimer(AA64Thread* thr, int which, itimerval_t* curr_value) {
	if(which != ITIMER_REAL) return -1;
	if (!curr_value) return -1;
	uint64_t now = _get_current_us();

	for (int i = 0; i < AURORA_MAX_TIMER; i++) {
		if (_timers[i].active && _timers[i].owner == thr &&
			_timers[i].handler == __kernel_timer_alarm_callback) {
			uint64_t remaining = (_timers[i].expireUS > now) ? (_timers[i].expireUS - now) : 0;
			_us_to_timeval(remaining, &curr_value->it_value);
			_us_to_timeval(_timers[i].intervalUS, &curr_value->it_interval);
			return 0;
		}
	}

	curr_value->it_value.tv_sec = curr_value->it_value.tv_usec = 0;
	curr_value->it_interval.tv_sec = curr_value->it_interval.tv_usec = 0;
	return 0;
}

