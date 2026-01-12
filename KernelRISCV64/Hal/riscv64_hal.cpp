/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#include "riscv64_hal.h"
#include "riscv64_lowlevel.h"
#include <aucon.h>

extern "C" void RISCV64CpuInitialize() {
    // Set up Trap Vector
    extern void riscv64_trap_vector_entry();
    uint64_t vector_addr = (uint64_t)&riscv64_trap_vector_entry;
    
    // Set stvec to point to our handler. Mode 0 (Direct) or 1 (Vectored).
    // Using Direct mode (all traps go to same address) for now. Last 2 bits 0.
    write_stvec(vector_addr & ~3ULL);
    
    // Enable software interrupts or timer if needed?
    // For now just basic init.
    
    AuTextOut("RISC-V 64 CPU Initialized\n");
}

extern "C" void RISCV64CPUPostInitialize(KERNEL_BOOT_INFO* info) {
    // Post init actions
}

extern "C" void AuRISCV64BoardInitialize() {
    // Board specific init
}


extern "C" void AuRISCV64TrapHandler(void* stack_frame) {
    uint64_t scause = read_scause();
    uint64_t sepc = read_sepc();
    
    AuTextOut("Trap Exception! \n");
    // TODO: Handle exception
    _hang();
}
