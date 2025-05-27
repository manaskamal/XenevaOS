/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
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

#include <stdint.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Hal/AA64/aa64cpu.h>
#include <Hal/AA64/vector.h>
#include <aucon.h>


uint64_t basisTime;
uint64_t cpuFrequency;

uint64_t AA64GetPhysicalTimerCount() {
	uint64_t val = get_cntpct_el0();
	return val * 100;
}

/*
 * AA64FPUNeonEnable -- initialize FPU and
 * NEON feature
 */
void AA64FPUNeonEnable() {
	uint64_t cpacr_el1 = get_cpacr_el1();
	cpacr_el1 |= (3 << 20) | (3 << 16);
	set_cpacr_el1(cpacr_el1);
}

/*
 * AA64ClockInitialize -- initialize the clock
 */
void AA64ClockInitialize() {
	uint64_t val = get_cntfrq_el0();
	cpuFrequency = val / 10000;

	basisTime = AA64GetPhysicalTimerCount() / cpuFrequency;
	AuTextOut("[aurora]: using %d mHZ \n", cpuFrequency);

}
/*
 * AA64CpuInitialize -- initialize aa64 cpu
 */
void AA64CpuInitialize() {
	cpuFrequency = 0;
	basisTime = 0;

	/*
	 * Initialize the vector table
	 */
	AA64InitializeVectorTable();

	/* Initialize the clock */
	AA64ClockInitialize();

	/* enable FPU and NEON */
	AA64FPUNeonEnable();

	// 
	//initialize Interrupt controller GIC 
	

	//if smp, set per core datas
}