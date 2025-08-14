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
#include <Drivers/uart.h>
#include <dtb.h>


GIC __gic;
uint32_t* gic_regs;
uint32_t* gicc_regs;

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
#define ISPENDING0 0x200


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


#define MAX_SPIS 1024

static uint8_t spiBitMap[MAX_SPIS];


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

	volatile uint8_t* mmio = (volatile uint8_t*)((uint64_t)mmio_ + reg);
	*mmio = value;
}

/*
* gic_inb_ -- reads a value from mmio register in byte
* @param reg -- register
*/
uint8_t gic_inb_(uint64_t* mmio_, int reg) {
	volatile uint8_t* mmio = (volatile uint8_t*)((uint64_t)mmio_ + reg);
	return *mmio;
}

GIC* AuGetSystemGIC() {
	return &__gic;
}

/*
 * AuGICGetMSIAddress -- calculate and return MSI address
 * for given spi offset
 * @param interruptID -- spi offset
 */
uint64_t AuGICGetMSIAddress(int interruptID) {
	UARTDebugOut("MSI Address : %x \n", __gic.gicMSIPhys + 0x040);
	return (__gic.gicMSIMMIO + 0x0040);
}

uint32_t AuGICGetMSIData(int interruptID) {
	UARTDebugOut("GICGetMSIData: %d \n", interruptID);
	return interruptID;
}

/*
 * AuGICAllocateSPI -- allocates Shared Peripheral 
 * interrupt ID 
 */
int AuGICAllocateSPI() {
	for (uint32_t i = 0; i < __gic.spiCount; i++) {
		if (spiBitMap[i] == 0) {
			spiBitMap[i] = 1;
			UARTDebugOut("[GIC]: Allocating SPI : %d , SPI_BASE : %d \n", i, __gic.spiBase);
			return __gic.spiBase + i;
		}
	}
	return -1;
}

/*
 *AuGICDeallocateSPI -- free up an used SPI id 
 *@param spiID -- target spi id
 */
void AuGICDeallocateSPI(int spiID) {
	if (spiID < __gic.spiBase || spiID >= (__gic.spiBase + __gic.spiCount))
		return;
	uint32_t index = spiID - __gic.spiBase;
	spiBitMap[index] = 0;
}

#define GICV2M_MSI_TYPER  0x008
#define GICV2M_MSI_SETSPI_NS 0x040
#define GICV2M_MSI_IIDR  0xFCC
void GICv2MInitialize() {
	uint32_t typer = *(volatile uint32_t*)__gic.gicMSIPhys;
	
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
				uint32_t addrS = AuDeviceTreeGetAddressCells(ic);
				uint32_t sizeS = AuDeviceTreeGetSizeCells(ic);
				AuTextOut("GIC ADr: %x \n", AuDeviceTreeGetRegAddress(ic, addrS));
				AuTextOut("GIC Szs: %x \n", AuDeviceTreeGetRegSize(ic, addrS, sizeS));
				AuTextOut("GICD : %x, GICC : %x \n", gicd, gicc);
				AuTextOut("GIC Addrsz : %d, sizesz : %d \n", addrS, sizeS);
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
	__gic.gicCMMIO = AuMapMMIO(__gic.gicCPhys, 2);
	__gic.gicMSIMMIO = AuMapMMIO(__gic.gicMSIPhys, 2);
	gic_regs = (volatile uint32_t*)__gic.gicDMMIO;
	gicc_regs = (volatile uint32_t*)__gic.gicCMMIO;

	uint32_t typer = gic_inl_(GICD(__gic), GICD_TYPER);
	uint32_t numirq = ((typer & 0x1f) + 1) * 32;
	AuTextOut("GIC Number of supported IRQ -> %d \n", numirq);

	uint32_t icc_iidr = gic_inl_(GICD(__gic), GICD_IIDR);
	uint32_t rev = (icc_iidr >> 16) & 0xFF;
	
	for (int i = 0; i < (numirq / 32); i++)
		GICD_ICENABLE(i) = 0xFFFFFFFF;

	uint32_t val = gic_inl_(GICD(__gic), GICD_CTLR);
	if ((val & 1))
		AuTextOut("GIC already enabled \n");

	

	/* enable the cpu interface*/
	/* writing 0xff means accepting all types of priority 0x0 -- 0xFF */
	gic_outl_(__gic.gicCMMIO, GICC_PMR, 0x1ff);
	gic_outl_(__gic.gicCMMIO, GICC_CTLR,0x1);
	isb_flush();

	/* enable the distributor interface */
	gic_outl_(__gic.gicDMMIO, GICD_CTLR, 0x1);
	isb_flush();

	for (int i = 0; i < MAX_SPIS; i++)
		spiBitMap[i] = 0;

	AuTextOut("GIC Initialized \n");
}

/* GICEnableIRQ -- enable an IRQ
 * @param irq -- IRQ number
 */
void GICEnableIRQ(uint32_t irq) {
	uint32_t reg = irq / 32;
	uint32_t bit = irq % 32;
	//GICClearPendingIRQ(irq);
	uint32_t icfgr = GICD_ICFGR(irq / 16);
	uint32_t shift = (irq % 16) * 2;
	uint32_t config_bits = (icfgr >> shift) & 0b11;
	if (config_bits == 0b00) {
		UARTDebugOut("IRQ : %d is level-sensitive \n", irq);
	}
	else if (config_bits == 0b10) {
		UARTDebugOut("IRQ : %d is edge-trigggered \n", irq);
	}
	else {
		UARTDebugOut("IRQ: %d is reserved or invalid config : %x \n",irq, config_bits);
	}
	//GICD_IGROUPR(reg) |= (1u << bit);
	/*uint8_t* gicd_itargetsr = (uint8_t*)GICD_ITARGETSR(irq);
	*gicd_itargetsr = 0x01;*/
	UARTDebugOut("ICFGR : %x \n", icfgr);
	*(volatile uint8_t*)(GICD(__gic) + GICD_IPRIORITYR(irq)) = 0xA0;
	GICD_ISENABLER(reg) |= (1u << bit);
}

void GICSetTargetCPU(int spi) {
	uint32_t reg_index = spi / 4;
	uint32_t byteShift = spi % 4;

	uint32_t val = GICD_ITARGETSR(reg_index);
	uint8_t cpu_mask = 1 << 0x00; //cpu0
	val &= ~(0xFF << (byteShift * 8));
	val |= (cpu_mask << (byteShift * 8));
	GICD_ITARGETSR(reg_index) = val;
}
void GICClearPendingIRQ(uint32_t irq) {
	uint32_t bit = irq % 32;
	volatile uint32_t* reg = (volatile uint32_t*)(GICD(__gic) + GICD_ICPENDR(irq));
	//UARTDebugOut("Clear pending reg : %x \n", (GICD(__gic) + GICD_ICPENDR(irq)));
	*reg |= (1U << bit);
}

void GICCheckPending(uint32_t irq) {
	uint32_t pend = *(volatile uint32_t*)(GICD(__gic) + ISPENDING0);
	if (pend & (1 << irq)) {
		//UARTDebugOut("IRQ : %d is still pending \n", irq);
	}
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
#define GICV2_CPUID_SHIFT 10
	//UARTDebugOut("GICEOI REG : %x \n", (GICC(__gic) + 0x10));
	//GICClearPendingIRQ(irqnum);
	*(volatile uint32_t*)(GICC(__gic) + 0x10) = irqnum;
	//UARTDebugOut("3N : %d \n", (*(volatile uint32_t*)(GICC(__gic) + 0x10)));
	dsb_ish();
	isb_flush();
}

void GICSetupTimer() {
	mask_irqs();
	setupTimerIRQ();
	gic_regs[0] = 1;
	gicc_regs[0] = 1;
	gicc_regs[1] = 0x1FF;

	gic_regs[64] = 0xFFFFffff;
	gic_regs[160] = 0xFFFFffff;
	gic_regs[65] = 0xFFFFFFFF;
	gic_regs[66] = 0xFFFFFFFF;
	gic_regs[67] = 0xFFFFFFFF;
	gic_regs[520] = 0x07070707;
	gic_regs[521] = 0x07070707;
	gic_regs[543] = 0x07070707;
}



