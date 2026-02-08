/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2025, Manas Kamal Choudhury
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
 **/

#include <dtb.h>
#include <Mm/vmmngr.h>
#include <stdint.h>
#include <aucon.h>
#include <string.h>

/* PLIC Memory Map (Standard) */
#define PLIC_PRIORITY_OFFSET    0x000000
#define PLIC_PENDING_OFFSET     0x001000
#define PLIC_ENABLE_OFFSET      0x002000
#define PLIC_THRESHOLD_OFFSET   0x200000
#define PLIC_CLAIM_OFFSET       0x200004

/* Global PLIC Base (Virtual) */
static uintptr_t _plic_base = 0;

/* Context for Hart 0 S-Mode */
/* Usually Context 0 = M-Mode, Context 1 = S-Mode on QEMU Virt */
#define PLIC_CTX_S_MODE  1

void AuPlicInitialize() {
    AuTextOut("[PLIC] Looking for PLIC in Device Tree...\r\n");
    
    /* 1. Find the PLIC Node */
    /* Search for "plic" or "interrupt-controller" with "riscv,plic0" */
    uint32_t* plicNode = AuDeviceTreeGetNode("plic");
    if (!plicNode) {
        /* Try checking by compatible string logic if name varies? 
           For now assume "plic" or "plic@..." works with current dtb.c
        */
        plicNode = AuDeviceTreeGetNode("interrupt-controller"); 
         /* Warning: This might find CPU intc. We need the one with 'reg' */
    }

    if (!plicNode) {
        AuTextOut("[PLIC] Error: PLIC Node not found in DTB!\r\n");
        return;
    }

    /* 2. Get Physical Address and Size */
    /* dtb.c helpers need address cells from parent? Usually root = 2 */
    uint32_t a_cells = 2; // Standard for 64-bit virt
    uint32_t s_cells = 2;
    
    uint64_t phys_base = AuDeviceTreeGetRegAddress(plicNode, a_cells);
    uint64_t size = AuDeviceTreeGetRegSize(plicNode, a_cells, s_cells);

    if (phys_base == 0) {
        AuTextOut("[PLIC] Error: Failed to extract reg property.\r\n");
        return;
    }

    AuTextOut("[PLIC] Found at Phys: 0x%lx, Size: 0x%lx\r\n", phys_base, size);

    /* 3. Map to Virtual Memory */
    size_t pages = size / PAGE_SIZE;
    if (size % PAGE_SIZE) pages++;
    
    _plic_base = (uintptr_t)AuMapMMIO(phys_base, pages);
    AuTextOut("[PLIC] Mapped to Virt: 0x%lx\r\n", _plic_base);

    /* 4. Initialization */
    /* Set Priority for all interrupts (1..1023) to 1 */
    volatile uint32_t* priorities = (volatile uint32_t*)(_plic_base + PLIC_PRIORITY_OFFSET);
    for (int i = 1; i < 1024; i++) {
        priorities[i] = 1; 
    }

    /* Enable Interrupts for Context 1 (S-Mode) */
    /* Enable Registers are at 0x2000 + (ctx * 0x80) */
    /* Each 32-bit register controls 32 interrupts */
    /* We enable first 32 words (1024 interrupts) */
    volatile uint32_t* enables = (volatile uint32_t*)(_plic_base + PLIC_ENABLE_OFFSET + (PLIC_CTX_S_MODE * 0x80));
    
    for (int i = 0; i < 32; i++) {
        enables[i] = 0xFFFFFFFF;
    }

    /* Set Priority Threshold to 0 (Permit all) for Context 1 */
    /* Threshold register at 0x200000 + (ctx * 0x1000) */
    volatile uint32_t* threshold = (volatile uint32_t*)(_plic_base + PLIC_THRESHOLD_OFFSET + (PLIC_CTX_S_MODE * 0x1000));
    *threshold = 0;

    AuTextOut("[PLIC] Initialized for Context %d (S-Mode).\r\n", PLIC_CTX_S_MODE);
}

/* 
 * AuPlicClaim -- Claim an external interrupt
 * Returns the interrupt ID (Source ID)
 */
uint32_t AuPlicClaim() {
    if (!_plic_base) return 0;
    
    volatile uint32_t* claim_reg = (volatile uint32_t*)(_plic_base + PLIC_CLAIM_OFFSET + (PLIC_CTX_S_MODE * 0x1000));
    return *claim_reg;
}

/*
 * AuPlicComplete -- Signal completion of interrupt
 * @param id -- Interrupt ID retrieved from Claim
 */
void AuPlicComplete(uint32_t id) {
    if (!_plic_base) return;
    
    volatile uint32_t* complete_reg = (volatile uint32_t*)(_plic_base + PLIC_CLAIM_OFFSET + (PLIC_CTX_S_MODE * 0x1000));
    *complete_reg = id;
}
