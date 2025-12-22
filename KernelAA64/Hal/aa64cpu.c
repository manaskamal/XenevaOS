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
#include <Hal/AA64/gic.h>
#include <aucon.h>
#include <Hal/basicacpi.h>
#include <Drivers/uart.h>
#include <Drivers/rtcmmio.h>
#include <kernelAA64.h>


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
	AuTextOut("[aurora]: using %d mHZ \r\n", cpuFrequency);
}

#define CACHE_LINE_SIZE 64
void aa64_data_cache_clean_range(void* addr, size_t size) {
	size_t start = (size_t)addr;
	size_t end = start + size;

	start &= ~(CACHE_LINE_SIZE - 1);
	for (size_t p = start; p < end; p += CACHE_LINE_SIZE)
		data_cache_flush((uint64_t*)p);
}

void AA64CPUImplementer(uint32_t midr) {
	uint8_t iid = (midr >> 24) & 0xFF;
	uint8_t arch = (midr >> 16) & 0xF;
	switch (iid) {
	case CPU_IMPLEMENTER_ARM:
		AuTextOut("CPU Implementer: ARM Limited \r\n");
		break;
	case CPU_IMPLEMENTER_BROADCOM:
		AuTextOut("CPU Implementer: Broadcom \r\n");
		break;
	case CPU_IMPLEMENTER_CAVIUM:
		AuTextOut("CPU Implementer: Cavium \r\n");
		break;
	case CPU_IMPLEMENTER_FUJITSU:
		AuTextOut("CPU Implementer: Fujitsu \r\n");
		break;
	case CPU_IMPLEMENTER_INTEL:
		AuTextOut("CPU Implementer: Intel \r\n");
		break;
	case CPU_IMPLEMENTER_APPLIED_MICRO:
		AuTextOut("CPU Implementer: Applied Micro \r\n");
		break;
	case CPU_IMPLEMENTER_QUALCOMM:
		AuTextOut("CPU Implementer: Qualcomm \r\n");
		break;
	case CPU_IMPLEMENTER_MARVELL:
		AuTextOut("CPU Implementer: Marvell \r\n");
		break;
	case CPU_IMPLEMENTER_APPLE:
		AuTextOut("CPU Implementer: Apple \r\n");
		break;
	default:
		AuTextOut("CPU Implementer: Unknown \r\n");
		break;
	}
	AuTextOut("CPU Architecture: ARMv8-A(%x) \r\n", arch);
}

extern void enableAlignCheck();
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

	/* because little boot always enable align check 
	 * so skip this
	 */
	if (!AuLittleBootUsed())
		enableAlignCheck();

	uint32_t id = read_midr();
	AA64CPUImplementer(id);

	enable_irqs();

	// 
	//initialize Interrupt controller GIC 
	

	//if smp, set per core datas
}

void AA64TimerSetup() {
#ifdef __TARGET_BOARD_RPI3__
	/* already enabled during bcm2836-armctl-ic initialization */
	return;
#endif
	setupTimerIRQ();
	GICEnableIRQ(27);
	GICClearPendingIRQ(27);
	isb_flush();
}

/*
 * AA64CPUPostInitialize -- initilaize post cpu requirements
 * @param info -- Pointer to KERNEL BOOT INFORMATIONs
 */
void AA64CPUPostInitialize(KERNEL_BOOT_INFO* info) {
	AuACPIInitialise(info->acpi_table_pointer);
	UARTInitialize();
	//enable_irqs();
	mask_irqs();
	suspendTimer();
	GICInitialize();
	AA64TimerSetup();
	//PS/2 Enable

	uint32_t id = read_midr();
	AA64CPUImplementer(id);
	AA64PCIeInitialize();
	AuTextOut("[aurora]: cpu post initialized \r\n");
	AuPL031RTCInit();
	mask_irqs();
}