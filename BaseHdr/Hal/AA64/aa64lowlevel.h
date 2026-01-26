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

#ifndef __AA64_LOW_LEVEL_H__
#define __AA64_LOW_LEVEL_H__

#include <aurora.h>

/*
 * _getCurrentEL -- returns the current exception level
 */
AU_EXTERN AU_EXPORT uint64_t _getCurrentEL();

/*
 * read_ttbr0_el1 -- reads the current exception's
 * page mapping base address
 */
AU_EXTERN AU_EXPORT uint64_t read_ttbr0_el1();

/*
 * write_ttbr0_el1 -- writes current page mapping base
 * address
 */
AU_EXTERN AU_EXPORT void write_ttbr0_el1(uint64_t* base);

AU_EXTERN AU_EXPORT uint64_t read_ttbr1_el1();

AU_EXTERN AU_EXPORT void write_ttbr1_el1(uint64_t* base);

extern void write_both_ttbr(uint64_t physicalBase);

AU_EXTERN AU_EXPORT uint64_t read_tcr_el1();

AU_EXTERN AU_EXPORT void write_tcr_el1(uint64_t tcr);

AU_EXTERN AU_EXPORT uint64_t read_mair_el1();

AU_EXTERN AU_EXPORT void write_mair_el1(uint64_t mair);

/*
 * read_sctlr_el1 -- reads current system control
 * register of el1
 */
AU_EXTERN AU_EXPORT uint64_t read_sctlr_el1();

/*
 * write_sctlr_el1 -- write current system control
 * register of el1
 * @param sctlr -- system control register value
 */
AU_EXTERN AU_EXPORT void write_sctlr_el1(uint64_t sctlr);

/*
 * dsb_ish -- data syncronization barrier inner shareablity memory
 * flush, which makes the previous memory access visible for
 * multi core system
 */
AU_EXTERN AU_EXPORT void dsb_ish();

AU_EXTERN AU_EXPORT void dmb_ish();

AU_EXTERN AU_EXPORT void dmb_sy();

AU_EXTERN AU_EXPORT void dsb_sy_barrier();

AU_EXTERN AU_EXPORT void set_kstack(uint64_t stack);

/*
 * isb_flush -- instruction synchronization flush, flush previous
 * instruction set
 */
AU_EXTERN AU_EXPORT void isb_flush();

AU_EXTERN AU_EXPORT void tlb_flush(uint64_t virtul_addr);

AU_EXTERN AU_EXPORT void tlb_flush_vmalle1is();
/*
 * set_vbar_el1 -- set vector base address for el1
 * @param addr -- address of the vector table
 */
AU_EXTERN AU_EXPORT void set_vbar_el1(uint64_t addr);

AU_EXTERN AU_EXPORT uint64_t get_cntpct_el0();

AU_EXTERN AU_EXPORT uint64_t get_cntfrq_el0();

AU_EXTERN AU_EXPORT uint64_t get_cpacr_el1();

extern uint64_t get_cntv_ctl_el0();

AU_EXTERN AU_EXPORT void set_cpacr_el1(uint64_t val);

AU_EXTERN AU_EXPORT uint64_t read_esr_el1();

AU_EXTERN AU_EXPORT uint64_t read_far_el1();

AU_EXTERN AU_EXPORT uint64_t read_elr_el1();

extern void aa64_write_sysreg(const char* reg, uint64_t value);

extern uint64_t aa64_read_sysreg(const char* reg);

extern void enable_sre();

extern uint32_t read_icc_iidr();

extern void set_cntp_cval_el0(uint64_t val);

extern void set_cntp_ctl_el0(uint64_t val);


extern void setupTimerIRQ();
extern void suspendTimer();
extern uint64_t readTimerCtl();

extern uint64_t read_spsr_el1();

extern void enable_irqs();

extern void mask_irqs();

extern uint32_t read_icc_iar1_el1();

extern uint32_t read_midr();
extern uint64_t read_mpidr_el1();
extern uint64_t read_daif();
extern uint64_t read_spsel();

extern void aa64_enter_user(uint64_t stack, uint64_t entryAddr);
extern void aa64_svc_test();
extern void aa64_utest();
extern void dc_ivac(uint64_t address);
extern void aa64_store_fp(uint8_t* address, uint64_t* fpcr, uint64_t* fpsr);
extern void aa64_restore_fp(uint8_t* address, uint64_t* fpcr, uint64_t* fpsr);
extern void data_cache_flush(uint64_t* address);
extern void aa64_data_cache_clean_range(void* addr, size_t size);
#endif