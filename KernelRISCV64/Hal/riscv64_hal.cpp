/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#include "riscv64_hal.h"
#include "riscv64_lowlevel.h"
#include "sbi.h"
#include <aucon.h>

#define TIMER_FREQ 10000000
#define TIMER_DT   100000  /* 10ms if freq is 10MHz */

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
    // Enable supervisor software, timer, and external interrupts
    uint64_t sie = read_sie();
    sie |= (1 << 1);  // SSIE - Supervisor Software Interrupt Enable
    sie |= (1 << 5);  // STIE - Supervisor Timer Interrupt Enable  
    sie |= (1 << 9);  // SEIE - Supervisor External Interrupt Enable
    write_sie(sie);
    
    /* Set initial timer */
    sbi_set_timer(read_time() + TIMER_DT);
    
    AuTextOut("[aurora]: RISC-V CPU post-initialization complete\r\n");
}

/* Forward decl for PLIC */
extern "C" void AuPlicInitialize();
extern "C" uint32_t AuPlicClaim();
extern "C" void AuPlicComplete(uint32_t id);

extern "C" void AuRISCV64BoardInitialize() {
    // Board specific init (QEMU virt)
    // PLIC and other peripherals would be initialized here
    AuPlicInitialize();
    AuTextOut("[aurora]: RISC-V board initialization complete\r\n");
}

/* RISC-V scause exception codes */
#define SCAUSE_INTERRUPT_BIT    (1ULL << 63)

/* Interrupt causes (scause bit 63 = 1) */
#define IRQ_S_SOFTWARE          1
#define IRQ_S_TIMER             5
#define IRQ_S_EXTERNAL          9

/* Exception causes (scause bit 63 = 0) */
#define EXC_INST_MISALIGNED     0
#define EXC_INST_ACCESS_FAULT   1
#define EXC_ILLEGAL_INST        2
#define EXC_BREAKPOINT          3
#define EXC_LOAD_MISALIGNED     4
#define EXC_LOAD_ACCESS_FAULT   5
#define EXC_STORE_MISALIGNED    6
#define EXC_STORE_ACCESS_FAULT  7
#define EXC_ECALL_FROM_U        8
#define EXC_ECALL_FROM_S        9
#define EXC_INST_PAGE_FAULT     12
#define EXC_LOAD_PAGE_FAULT     13
#define EXC_STORE_PAGE_FAULT    15

static const char* riscv64_exception_names[] = {
    "Instruction address misaligned",
    "Instruction access fault",
    "Illegal instruction",
    "Breakpoint",
    "Load address misaligned",
    "Load access fault",
    "Store address misaligned",
    "Store access fault",
    "Environment call from U-mode",
    "Environment call from S-mode",
    "Reserved",
    "Reserved",
    "Instruction page fault",
    "Load page fault",
    "Reserved",
    "Store page fault"
};

/* Forward declaration for scheduler */
extern "C" uint64_t AuScheduleThread(uint64_t stack_frame);

extern "C" uint64_t AuRISCV64TrapHandler(void* stack_frame) {
    uint64_t scause = read_scause();
    uint64_t sepc = read_sepc();
    uint64_t stval = read_stval();
    uint64_t ret_stack = (uint64_t)stack_frame;
    
    bool is_interrupt = (scause & SCAUSE_INTERRUPT_BIT) != 0;
    uint64_t cause_code = scause & 0x7FFFFFFFFFFFFFFFULL;
    
    if (is_interrupt) {
        /* Handle interrupts */
        switch (cause_code) {
            case IRQ_S_SOFTWARE:
                /* Supervisor software interrupt */
                /* Clear the pending bit */
                write_sip(read_sip() & ~(1 << 1));
                /* Could be used for IPI or scheduler trigger */
                break;
                
            case IRQ_S_TIMER:
                /* Supervisor timer interrupt */
                /* Reload timer for next tick */
                sbi_set_timer(read_time() + TIMER_DT);
                
                /* This is where the scheduler would be called */
                ret_stack = AuScheduleThread((uint64_t)stack_frame);
                break;
                
            case IRQ_S_EXTERNAL:
                /* Supervisor external interrupt (from PLIC) */
                {
                    uint32_t irq = AuPlicClaim();
                    // AuTextOut("[IRQ] External interrupt: %d\r\n", irq);
                    
                    if (irq) {
                        /* TODO: Dispatch to driver */
                        AuPlicComplete(irq);
                    }
                }
                break;
                
            default:
                AuTextOut("[IRQ] Unknown interrupt: %d\r\n", cause_code);
                break;
        }
    } else {
        /* Handle exceptions */
        const char* exc_name = "Unknown exception";
        if (cause_code < 16) {
            exc_name = riscv64_exception_names[cause_code];
        }
        
        switch (cause_code) {
            case EXC_ECALL_FROM_U:
                /* System call from user mode */
                /* a7 contains syscall number, a0-a5 are arguments */
                /* For now, just advance sepc past ecall (4 bytes) */
                AuTextOut("[User Mode]: Syscall trapped! (SVC: %d)\r\n", ((uint64_t*)stack_frame)[128/8]); // a7 is at offset 120? No.
                // Frame: ra=0, sp=8... a7=120?
                // store_a0_a7 stores a1 at 0... wait.
                // arch_context_switch stores: ra=0, s0=8...
                // riscv64_trap_vector_entry:
                // sp -= 256
                // ra=0, t0=8 ... a0=64 ... a7=120.
                AuTextOut("[User Mode]: Syscall trapped! (a7=%d)\r\n", ((uint64_t*)stack_frame)[15]); // 120/8 = 15
                write_sepc(sepc + 4);
                return ret_stack;  /* Return normally */
                
            case EXC_ECALL_FROM_S:
                /* System call from supervisor mode */
                /* This is used for AuForceScheduler (yield) */
                /* Advance SEPC in saved frame (offset 128 / 8 = 16) */
                ((uint64_t*)stack_frame)[16] += 4;
                ret_stack = AuScheduleThread((uint64_t)stack_frame);
                return ret_stack;
                
            case EXC_BREAKPOINT:
                /* Breakpoint - advance past ebreak */
                write_sepc(sepc + 4);
                AuTextOut("[DEBUG] Breakpoint hit\r\n");
                return ret_stack;
            
            case EXC_INST_PAGE_FAULT:
            case EXC_LOAD_PAGE_FAULT:
            case EXC_STORE_PAGE_FAULT:
                 AuTextOut("\r\n=== RISC-V Exception ===\r\n");
                 AuTextOut("Cause: %d (%s)\r\n", cause_code, exc_name);
                 AuTextOut("SEPC:  %x\r\n", sepc);
                 AuTextOut("STVAL: %x\r\n", stval);
                /* Page fault - would need to handle demand paging here */
                AuTextOut("[FAULT] Page fault at address: %x\r\n", stval);
                /* For now, fatal */
                break;
                
            case EXC_ILLEGAL_INST:
                 AuTextOut("\r\n=== RISC-V Exception ===\r\n");
                 AuTextOut("Cause: %d (%s)\r\n", cause_code, exc_name);
                 AuTextOut("SEPC:  %x\r\n", sepc);
                 AuTextOut("STVAL: %x\r\n", stval);
                AuTextOut("[FAULT] Illegal instruction at: %x\r\n", sepc);
                break;
                
            default:
                 AuTextOut("\r\n=== RISC-V Exception ===\r\n");
                 AuTextOut("Cause: %d (%s)\r\n", cause_code, exc_name);
                 AuTextOut("SEPC:  %x\r\n", sepc);
                 AuTextOut("STVAL: %x\r\n", stval);
                break;
        }
        
        /* Fatal exception - halt */
        AuTextOut("=== System Halted ===\r\n");
        _hang();
    }
    
    return ret_stack;
}

