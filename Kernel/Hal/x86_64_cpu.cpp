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

#include <Hal/x86_64_cpu.h>
#include <Hal/pcpu.h>
#include <Hal/x86_64_gdt.h>
#include <Hal/x86_64_idt.h>
#include <Hal/x86_64_lowlevel.h>
#include <Hal/apic.h>
#include <Hal/x86_64_ap_init.h>
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <aucon.h>

TSS* _tss;
bool _fxsave = false;

/*
 * x86_64_enable_syscall_ext -- enablse syscall 
 * extension
 */
void x86_64_enable_syscall_ext() {
	size_t efer = x64_read_msr(IA32_EFER);
	efer |= (1 << 11);
	efer |= 1;
	x64_write_msr(IA32_EFER, efer);
}

/*
 * x86_64_init_user -- initialise the user land
 * @param bit -- either 32 or 64 bit
 */
void x86_64_init_user(size_t bit) {
	uint16_t data_sel = SEGVAL(GDT_ENTRY_USER_DATA, 3);
	uint16_t code_sel = 0;
	switch (bit) {
	case 64:
		code_sel = SEGVAL(GDT_ENTRY_USER_CODE, 3);
		break;
	case 32:
		code_sel = SEGVAL(GDT_ENTRY_USER_CODE32, 3);
		break;
	default:
		return;
	}

	gdtr peek_gdt;
	x64_sgdt(&peek_gdt);
	gdt_entry& tss_entry = peek_gdt.gdtaddr[GDT_ENTRY_TSS];

	_tss = (TSS*)(tss_entry.base_low + (tss_entry.base_mid << 16) + (tss_entry.base_high << 24) +
		((uint64_t)*(uint32_t*)&peek_gdt.gdtaddr[GDT_ENTRY_TSS + 1] << 32));

}

/*
 * x86_64_init_user_ap -- initialise user land for application
 * processor's
 * @param bit -- cpu bit
 */
void x86_64_init_user_ap(size_t bit) {

	uint16_t data_sel = SEGVAL(GDT_ENTRY_USER_DATA, 3);
	uint16_t code_sel = 0;
	switch (bit) {
	case 64:
		code_sel = SEGVAL(GDT_ENTRY_USER_CODE, 3);
		break;
	case 32:
		code_sel = SEGVAL(GDT_ENTRY_USER_CODE32, 3);
		break;
	default:
		return;
	}

	gdtr peek_gdt;
	x64_sgdt(&peek_gdt);
	gdt_entry& tss_entry = peek_gdt.gdtaddr[GDT_ENTRY_TSS];

	TSS *tss_ = (TSS*)(tss_entry.base_low + (tss_entry.base_mid << 16) + (tss_entry.base_high << 24) + ((uint64_t)*(uint32_t*)&peek_gdt.gdtaddr[GDT_ENTRY_TSS + 1] << 32));

	AuPerCPUSetKernelTSS(tss_);
}

/*
 * x86_64_get_tss -- returns the tss
 */
TSS* x86_64_get_tss() {
	return _tss;
}

/*
 * x86_64_hal_cpu_feature_enable -- enable cpu features
 */
void x86_64_hal_cpu_feature_enable() {
	uint64_t cr0 = x64_read_cr0();
	cr0 &= ~(1 << 2);
	cr0 |= (1 << 1);
	x64_write_cr0(cr0);

	size_t a, b, c, d;
	x64_cpuid(1, &a, &b, &c, &d, 0);
	if ((c & (1 << 26)) != 0) {
		uint64_t cr4 = x64_read_cr4();
		cr4 |= (1 << 18);
		x64_write_cr4(cr4);
	}

	if ((d & (1 << 25)) != 0) {
		size_t cr4 = x64_read_cr4();

		if ((d & (1 << 24)) != 0) {
			cr4 |= (1 << 9);
			_fxsave = true;
		}

		cr4 |= (1 << 10);
		x64_write_cr4(cr4);
	}

	else if ((d & (1 << 26)) != 0) {
		//supported SSE2
	}

	else if ((c & (1 << 0)) != 0) {
		//supported SSE3
	}
}


bool x86_64_is_cpu_fxsave_supported() {
	return _fxsave;
}

/*
 * x86_64_cpu_msi_address -- calculates the cpu msi address
 * @param data -- msi data to return
 * @param vector -- interrupt vector number
 * @param processor -- processor number
 * @param edge -- edge triggered or level triggered
 * @param deassert -- deassert bit
 */
uint64_t x86_64_cpu_msi_address(uint64_t* data, size_t vector, uint32_t processor, uint8_t edge, uint8_t deassert) {
	*data = (vector & 0xFF) | (edge == 1 ? 0 : (1 << 15)) | (deassert == 1 ? 0 : (1 << 14));
	//*data = low;
	return (0xFEE00000 | (processor << 12));
}


bool __ApStarted;

uint64_t _ICRDest(uint32_t processor) {
	if (X2APICSupported())
		return ((uint64_t)processor << 32);
	else
		return ((uint64_t)processor << 56);
}

bool _ICRBusy() {
	return (ReadAPICRegister(LAPIC_REGISTER_ICR) & (1 << 12)) != 0;
}


/*
 * x86_64_cpu_initialize -- initialise all Application processor
 * except the BSP processor
 * @param num_cpu -- cpu number to initialise except 0
 */
void x86_64_cpu_initialize(uint8_t num_cpu) {
	if (num_cpu == 0)
		return;

	//! fixed address
	uint64_t *apdata = (uint64_t*)0xA000;
	uint64_t ap_init_address = (uint64_t)x86_64_ap_init;
	uint64_t ap_aligned_address = (uint64_t)apdata;

	uint64_t *pml4 = (uint64_t*)AuGetRootPageTable();

	for (int i = 1; i <= num_cpu; i++) {

		/* In SMP Mode : no more than 8 cpus */
		if (i == 8)
			break;
	
		__ApStarted = false;


		void *stack_address = AuPmmngrAlloc();
		*(uint64_t*)(ap_aligned_address + 8) = (uint64_t)pml4;
		*(uint64_t*)(ap_aligned_address + 16) = (uint64_t)stack_address;
		*(uint64_t*)(ap_aligned_address + 24) = ap_init_address;
		*(uint64_t*)(ap_aligned_address + 32) = (uint64_t)P2V((size_t)AuPmmngrAlloc());
		void* cpu_struc = (void*)P2V((uint64_t)AuPmmngrAlloc());
		CPUStruc *cpu = (CPUStruc*)cpu_struc;
		cpu->cpu_id = i;
		cpu->au_current_thread = 0;
		cpu->kernel_tss = 0;
		*(uint64_t*)(ap_aligned_address + 40) = (uint64_t)cpu_struc;


		WriteAPICRegister(LAPIC_REGISTER_ICR, _ICRDest(i) | 0x4500);
		while (_ICRBusy());


		size_t startup_ipi = _ICRDest(i) | 0x4600 | ((size_t)apdata >> 12);
		WriteAPICRegister(LAPIC_REGISTER_ICR, startup_ipi);
		while (_ICRBusy());
		for (int i = 0; i < 10000000; i++)
			;
		WriteAPICRegister(LAPIC_REGISTER_ICR, startup_ipi);
		while (_ICRBusy());

		for (int i = 0; i < 10000000; i++)
			;
		do {
			x64_pause();
		} while (!__ApStarted);
	}
}

/*
 * x86_64_set_ap_start_bit -- set a bit in __ApStarted
 * @param value -- bit to set
 */
void x86_64_set_ap_start_bit(bool value) {
	__ApStarted = value;
}

extern "C" void syscall_entry();
extern "C" void x64_syscall_entry_compat();

/*
 * x86_64_initialise_syscall -- initialise the syscall service
 */
void x86_64_initialise_syscall() {
	uint64_t syscall_sel = SEGVAL(GDT_ENTRY_KERNEL_CODE, 0);
	uint64_t sysret_sel = SEGVAL(GDT_ENTRY_USER_CODE32, 3);

	x64_write_msr(IA32_STAR, (sysret_sel << 48) | (syscall_sel << 32));
	x64_write_msr(IA32_LSTAR, (size_t)&syscall_entry);
	x64_write_msr(IA32_SFMASK, IA32_EFLAGS_INTR | IA32_EFLAGS_DIRF);
	x64_write_msr(IA32_CSTAR, (size_t)&x64_syscall_entry_compat);
}

/*
 * cpu_read_tsc -- read the tsc count
 */
uint64_t cpu_read_tsc(){
	uint32_t hi = 0;
	uint32_t lo = 0;
	x64_rdtsc(&hi, &lo);
	uint64_t count = (((uint64_t)hi << 32UL) | (uint64_t)lo);
	return count;
}