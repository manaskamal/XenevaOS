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


#include <Hal/AA64/gic.h>
#include <aucon.h>
#include <Mm/vmmngr.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <dtb.h>


GIC __gic;

/* distributor registers */
#define GICD(n)  (n.gicDMMIO)
#define GICD_CTLR    0x0000
#define GICD_TYPER   0x0004
#define GICD_IIDR    0x0008
#define GICD_ISENABLER(n) (*(volatile uint32_t*)(GICD(__gic) + 0x100 + 4*(n)))
#define GICD_IPRIORITYR(n) (0x400 + (n))
#define GICD_IGROUPR(n)  (*(volatile uint32_t*)(GICD(__gic) + 0x080 + 4*(n)))
#define GICD_ICENABLE(n) (*(volatile uint32_t*)(GICD(__gic) + 0x180 + (n*4)))
#define GICD_ICFGR(n) (*(volatile uint32_t*)(GICD(__gic) + 0x0C00 + (n*4)))
#define GICD_ITARGETSR(n) (*(volatile uint32_t*)(GICD(__gic) + 0x0800 + (n*4)))
#define GICD_IROUTER(n) (0x6000 + 8*(n))
#define GICD_ICPENDR(n) (0x280 + (n/32)*4)


/* cpu registers offsets */
#define GICC_CTLR  0x0000
#define GICC_PMR   0x0004
#define GICC_BPR   0x0008
#define GICC_IAR   0x000C
#define GICC_EOIR  0x0010
#define GICC_HPPIR 0x0018
#define GICC_ABPR  0x001C
#define GICC_IIDR  0x00FC
#define GICC(n)  (n.gicCMMIO)


#define GICR_CTLR  0x0000
#define GICR_IIDR  0x0004
#define GICR_WAKER 0x0014
#define GICR(n)  (n.gicRPhys)

uint64_t* gic_dist_mmio;
uint64_t* gic_redist_mmio;



/*
 * gic_outl_ -- writes a value to mmio register in dword
 * @param reg -- register
 * @param value -- value to write
 */
void gic_outl_(uint64_t* mmio_,int reg, uint32_t value) {
	volatile uint32_t* mmio = (volatile uint32_t*)((uint64_t)mmio_ + reg);
	*mmio = value;
}

/*
 * gic_inl_ -- reads a value from mmio register in dword
 * @param reg -- register
 */
uint32_t gic_inl_(uint64_t* mmio_, int reg) {
	volatile uint32_t* mmio = (volatile uint32_t*)((uint64_t)mmio_ + reg);
	return *mmio;
}

/*
 * gic_outw_ -- writes a value to mmio register in word
 * @param reg -- register
 * @param value -- value to write
 */
void gic_outw_(uint64_t* mmio_, int reg, uint16_t value) {
	volatile uint16_t* mmio = (volatile uint16_t*)(mmio_ + reg);
	*mmio = value;
}

/*
 * gic_inw_ -- reads a value from mmio register in word
 * @param reg -- register
 */
uint16_t gic_inw_(uint64_t* mmio_,int reg) {
	volatile uint16_t* mmio = (volatile uint16_t*)(mmio_ + reg);
	return *mmio;
}


/*
 * gic_outb_ -- writes a value to mmio register in byte
 * @param reg -- register
 * @param value -- value to write
 */
void gic_outb_(uint64_t* mmio_, int reg, uint8_t value) {

	volatile uint8_t* mmio = (volatile uint8_t*)((uintptr_t)mmio_ + reg);
	*mmio = value;
}

/*
* gic_inb_ -- reads a value from mmio register in byte
* @param reg -- register
*/
uint8_t gic_inb_(uint64_t* mmio_, int reg) {
	volatile uint8_t* mmio = (volatile uint8_t*)((uintptr_t)mmio_ + reg);
	return *mmio;
}

GIC* AuGetSystemGIC() {
	return &__gic;
}

/*
 * GICInitialize -- initialize the gic 
 * controller, it can be parsed through ACPI MADT 
 * or hard-coding
 */
void GICInitialize() {
	/* Firstly rely upon UEFI + ACPI for GIC mmio values */
	AuTextOut("Initializing GIC %x %x\n", __gic.gicCPhys, __gic.gicDPhys);

	/* if not found, go for device tree , because the kernel might get booted
	 * from LittleBoot 
	 */
	if (__gic.gicCPhys == 0 && __gic.gicDPhys == 0) {
		//Need to fall to Device Tree
		uint32_t* ic = AuDeviceTreeGetNode("intc@");
		if (!ic) {
			AuTextOut("Interrupt controller node not found \n");
		}
		else {
			uint32_t* compat = AuDeviceTreeFindProperty(ic, "compatible");
			fdt_property_t* prop = (fdt_property_t*)compat;
			if (compat) {
				AuTextOut("Interrupt controller found: %s \n", prop->value);
			}
			uint32_t* addressSz = AuDeviceTreeFindProperty(ic, "#address-cells");
			uint32_t* reg = AuDeviceTreeFindProperty(ic, "reg");
			prop = (fdt_property_t*)reg;
			if (reg) {
				/* suspecting, this would only work with QEMU-aarch64-virt board */
				uint32_t gicd = (uint64_t)AuDTBSwap32(reg[3]) | ((uint64_t)AuDTBSwap32(reg[4]) << 32UL);
				uint32_t gicc = (uint64_t)AuDTBSwap32(reg[7]) | ((uint64_t)AuDTBSwap32(reg[9]) << 32UL);
				AuTextOut("GICD : %x, GICC : %x \n", gicd, gicc);
				__gic.gicDPhys = gicd;
				__gic.gicCPhys = gicc;
			}

		}
	}

	if (__gic.gicCPhys == 0 && __gic.gicDPhys == 0) {
		AuTextOut("[aurora]: No GIC MMIO found\n");
		return;
	}

	__gic.gicDMMIO = AuMapMMIO(__gic.gicDPhys, 2);
	__gic.gicCMMIO = AuMapMMIO(__gic.gicCPhys, 1);

	uint32_t typer = gic_inl_(GICD(__gic), GICD_TYPER);
	uint32_t numirq = ((typer & 0x1f) + 1) * 32;
	AuTextOut("GIC Number of supported IRQ -> %d \n", numirq);

	for (int i = 0; i < (numirq / 32); i++)
		GICD_ICENABLE(i) = 0xFFFFFFFF;

	uint32_t val = gic_inl_(GICD(__gic), GICD_CTLR);
	if ((val & 1))
		AuTextOut("GIC already enabled \n");

	

	/* enable the cpu interface*/
	/* writing 0xff means accepting all types of priority 0x0 -- 0xFF */
	gic_outl_(__gic.gicCMMIO, GICC_PMR, 0xff);
	gic_outl_(__gic.gicCMMIO, GICC_CTLR,0x1);
	isb_flush();

	/* enable the distributor interface */
	gic_outl_(__gic.gicDMMIO, GICD_CTLR, 0x1);
	isb_flush();
	AuTextOut("GIC Initialized \n");
}

/* GICEnableIRQ -- enable an IRQ
 * @param irq -- IRQ number
 */
void GICEnableIRQ(uint32_t irq) {
	uint32_t reg = irq / 32;
	uint32_t bit = irq % 32;
	GICClearPendingIRQ(irq);
	GICD_ISENABLER(reg) |= (1 << bit);
}

void GICClearPendingIRQ(uint32_t irq) {
	uint32_t bit = irq % 32;
	volatile uint32_t* reg = (volatile uint32_t*)(GICD(__gic) + GICD_ICPENDR(irq));
	*reg = (1U << bit);
}

/*
 * GICReadIAR -- read interrupt acknowledge
 * register
 */
uint32_t GICReadIAR() {
	uint32_t iar = *(volatile uint32_t*)(GICC(__gic) + 0x0C);
	return iar;
}

/*
 * GICSendEOI --sends end of interrupt to 
 * GIC cpu interface
 */
void GICSendEOI(uint32_t irqnum) {
	*(volatile uint32_t*)(GICC(__gic) + 0x10) = irqnum;
}



