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

#include <Hal/AA64/rpi3.h>
#include <Mm/vmmngr.h>
#include <Drivers/uart.h>
#include <Mm/pmmngr.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <aucon.h>
#include <Board/RPI3bp/rpi3bp.h>
#include <dtb.h>

#ifdef __TARGET_BOARD_RPI3__

uint64_t lpbase;
#define PERIPHERAL_BASE  0x3F000000
#define LOCAL_PERIPHERAL_BASE 0x40000000

#define LOCAL_CONTROL  0x00
#define LOCAL_PRESCALER 0x08


#define CORE0_TIMER_IRQCNTL  0x40
#define CORE1_TIMER_IRQCNTL  0x44
#define CORE2_TIMER_IRQCNTL  0x48
#define CORE3_TIMER_IRQCNTL  0x4C

#define CORE0_MBOX_IRQCNTL 0x50
#define CORE1_MBOX_IRQCNTL 0x54
#define CORE2_MBOX_IRQCNTL 0x58
#define CORE3_MBOX_IRQCNTL 0x5C

#define CORE0_IRQ_SOURCE 0x60
#define CORE1_IRQ_SOURCE 0x64
#define CORE2_IRQ_SOURCE 0x68
#define CORE3_IRQ_SOURCE 0x6C

#define CORE0_FIQ_SOURCE 0x70
#define CORE1_FIQ_SOURCE  0x74
#define CORE2_FIQ_SOURCE 0x78
#define CORE3_FIQ_SOURCE 0x7C

#define LOCAL_GPU_INT_ROUTING 0x0C

/* peripheral base */
#define IRQ_BASIC_PENDING 0xb200
#define IRQ_PENDING_1 0xb204
#define IRQ_PENDING_2 0xb208
#define FIQ_CONTROL 0xb20C
#define ENABLE_IRQS_1 0xb210
#define ENABLE_IRQS_2 0xb214
#define ENABLE_BASIC_IRQS 0xb218
#define DISABLE_IRQS_1 0xb21C
#define DISABLE_IRQS_2 0xb220
#define DISABLE_BASIC_IRQS 0xb224

uint64_t* mbox;
uint64_t vcmbox_mmio;

#define MBOX_READ  ((volatile unsigned int*)(vcmbox_mmio + 0x0))
#define MBOX_POLL  ((volatile unsigned int*)(vcmbox_mmio + 0x10))
#define MBOX_SENDER ((volatile unsigned int*)(vcmbox_mmio + 0x14))
#define MBOX_STATUS ((volatile unsigned int*)(vcmbox_mmio + 0x18))
#define MBOX_CONFIG ((volatile unsigned int*)(vcmbox_mmio + 0x1C))
#define MBOX_WRITE  ((volatile unsigned int*)(vcmbox_mmio + 0x20))

#define ARMCTRL_READ(addr) (*(volatile uint32_t*)(addr))
#define ARMCTRL_WRITE(addr,val) (*(volatile uint32_t*)(addr)= (val))

/* 
 * AuRPI3GetCoreID -- get currently running core
 * number
 */
uint32_t AuRPI3GetCoreID() {
    uint64_t mpidr = read_mpidr_el1();
    return (mpidr & 0x3);
}

/*
 * AuRPI3LocalIrqInit -- initialize local interrupts, i.e
 * interrupts related to each core
 */
void AuRPI3LocalIrqInit() {
    uint32_t coreID = AuRPI3GetCoreID();
    uint64_t localPbase = (uint64_t)AuMapMMIO(LOCAL_PERIPHERAL_BASE, 1);
    lpbase = localPbase;

    AuTextOut("[aurora]: coreID : %d \r\n", coreID);
    AuTextOut("[aurora]: local pheripheral mapped to : %x \r\n", lpbase);

    /* set the prescaler value to divider ratio of 1, setting the prescaler to
     * zero will stop the timer
     */
    ARMCTRL_WRITE(lpbase + LOCAL_PRESCALER, 0x80000000);

    /* route all gpu interrupts to core 0*/
    ARMCTRL_WRITE(lpbase + LOCAL_GPU_INT_ROUTING, 0x00);

    /* initialize the core0 timer to reset value*/
    uint64_t timer_ctrl_addr = lpbase + CORE0_TIMER_IRQCNTL + (coreID * 4);
    uint64_t mbox_ctrl_addr = lpbase + CORE0_MBOX_IRQCNTL + (coreID * 4);

    ARMCTRL_WRITE(timer_ctrl_addr, 0);
    ARMCTRL_WRITE(mbox_ctrl_addr, 0);

    /* set the 64 bit core timer to run at crystal clock
       and increments by 1 step */
    ARMCTRL_WRITE(lpbase + LOCAL_CONTROL, (0ULL << 8) | (0ULL < 9));
}

/*
 * AuRPI3LocalTimerIRQEnable -- enable timer related to each
 * core
 * @param timer_mask -- which timer type to enable or mask
 */
void AuRPI3LocalTimerIRQEnable(uint32_t timer_mask) {
    uint32_t core_id = AuRPI3GetCoreID();
    uint64_t timer_ctlr = lpbase + CORE0_TIMER_IRQCNTL + (core_id * 4);
    uint32_t ctrl = ARMCTRL_READ(timer_ctlr);
    ctrl |= timer_mask;
    ARMCTRL_WRITE(timer_ctlr, ctrl);
}

/*
 * AuRPI3PeripheralIRQInit -- initialize peripheral irqs
 */
void AuRPI3PeripheralIRQInit() {
    ARMCTRL_WRITE(PERIPHERAL_BASE + DISABLE_IRQS_1, 0xFFFFFFFF);
    ARMCTRL_WRITE(PERIPHERAL_BASE + DISABLE_IRQS_2, 0xFFFFFFFF);
    ARMCTRL_WRITE(PERIPHERAL_BASE + DISABLE_BASIC_IRQS, 0xFFFFFFFF);
}

/*
 * AuRPI3PeripheralIRQEnable -- enable particular peripheral irq
 * @param irq_num -- irq number of the specific peripheral
 */
void AuRPI3PeripheralIRQEnable(uint32_t irq_num) {
    if (irq_num < 32) {
        ARMCTRL_WRITE(PERIPHERAL_BASE + ENABLE_IRQS_1, 1 << irq_num);
    }
    else if (irq_num < 64) {
        ARMCTRL_WRITE(PERIPHERAL_BASE + ENABLE_IRQS_2, 1 << (irq_num - 32));
    }
}

/*
 * AuRPI3PeripheralIRQDisable -- disable particular peripheral
 * irq
 * @param irq_num -- irq number of that peripheral
 */
void AuRPI3PeripheralIRQDisable(uint32_t irq_num) {
    if (irq_num < 32) {
        ARMCTRL_WRITE(PERIPHERAL_BASE + ENABLE_IRQS_1, 1 << irq_num);
    }
    else if (irq_num < 64) {
        ARMCTRL_WRITE(PERIPHERAL_BASE + ENABLE_IRQS_2, 1 << (irq_num - 32));
    }
}

/*
 * AuRPI3LocalIRQGetPending -- read pending interrupts from
 * specific core
 */
uint32_t AuRPI3LocalIRQGetPending() {
    uint32_t core_id = AuRPI3GetCoreID();
    uint64_t irq_src_addr = lpbase + CORE0_IRQ_SOURCE + (core_id * 4);
    return ARMCTRL_READ(irq_src_addr);
}

/*
 * AuRPI3TimerInit -- initialize and caliberate the
 * 64 bit periodic timer 
 */
void AuRPI3TimerInit(uint32_t interval_ms) {
    uint64_t freq = get_cntfrq_el0();
    uint64_t interval_ticks = (freq * interval_ms) / 1000;
    uint64_t current = get_cntpct_el0();
    set_cntp_cval_el0(current + interval_ticks);
    set_cntp_ctl_el0(1);
}

/* RPI3ICInit -- initialize RPI3 local interrupt controller */
void AuRPI3ICInit() {
    AuRPI3TimerInit(10);
    AuRPI3LocalIrqInit();
    AuRPI3PeripheralIRQInit();
    AuRPI3LocalTimerIRQEnable(INT_SRC_CNTPNSIRQ);
}
/*
 * AuRPI3Initialize -- initialize the basic board requirement
 */
void AuRPI3Initialize() {
    vcmbox_mmio = (uint64_t)AuMapMMIO(VIDEOCORE_MBOX, 1);
    uint64_t pa = (uint64_t)AuPmmngrAlloc();
    pa = (pa + 0xF) & ~0xFULL;
    mbox = (uint64_t*)P2V(pa);
    AuTextOut("[aurora]: Rasperry Pi 3b+ board initialized \r\n");
    AuTextOut("[aurora]: Video Core MBOX MMIO : %x \r\n", vcmbox_mmio);
}

#endif

