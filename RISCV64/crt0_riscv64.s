/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2023-2026, Manas Kamal Choudhury
 * All rights reserved.
 *
 * crt0_riscv64.s -- C runtime startup for RISC-V user space (XenevaOS)
 *
 * The kernel AuProcessEntUser calls our entrypoint with:
 *   a0 = argc
 *   a1 = argv pointer
 * and the stack pointer already set to the user stack.
 **/

.section .text
.align 2
.global _start
_start:
    /* a0 = argc, a1 = argv — passed from kernel */
    call main
    
    /* If main returns, call ProcessExit (syscall 5) */
    li a7, 5
    ecall

    /* Should never reach here */
1:  j 1b
