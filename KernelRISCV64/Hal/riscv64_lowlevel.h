/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#ifndef __RISCV64_LOWLEVEL_H__
#define __RISCV64_LOWLEVEL_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t read_satp();
void write_satp(uint64_t val);
uint64_t read_sstatus();
void write_sstatus(uint64_t val);
uint64_t read_sie();
void write_sie(uint64_t val);
uint64_t read_sip();
void write_sip(uint64_t val);
uint64_t read_stvec();
void write_stvec(uint64_t val);
uint64_t read_scause();
uint64_t read_stval();
uint64_t read_sepc();
void write_sepc(uint64_t val);
uint64_t read_sscratch();
void write_sscratch(uint64_t val);

void sfence_vma();
void enable_irq();
void disable_irq();

void arch_context_switch(void* prev_rsp_addr, void* next_rsp);

void _hang();

#ifdef __cplusplus
}
#endif

#endif
