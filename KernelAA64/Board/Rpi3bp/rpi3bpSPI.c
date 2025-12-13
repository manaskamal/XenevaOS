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

#include <Board/RPI3bp/rpi3bp_spi.h>
#include <Hal/AA64/rpi3.h>
#include <aucon.h>
#include <Board/RPI3bp/rpi3bp_gpio.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Mm/vmmngr.h>
#include <Mm/pmmngr.h>

static uint64_t* spi0Base;

#define SPI0_CS  0x00
#define SPI0_FIFO 0x04
#define SPI0_CLK 0x08

#define SPI_CS_TXD (1ULL<<18)
#define SPI_CS_RXD (1ULL<<17)
#define SPI_CS_DONE (1ULL<<16)
#define SPI_CS_TA (1ULL<<7)
#define SPI_CS_CLEAR_RX (1ULL<<5)
#define SPI_CS_CLEAR_TX (1ULL<<4)
#define SPI_CS_CS (3ULL<<0)
#define SPI_CS_CS0 (0ULL<<0)
#define SPI_CS_CS1 (1ULL<<0)
#define SPI_CS_CS2 (2ULL<<0)


#define SPI0_GPIO_DC 25
#define SPI0_GPIO_RST 24 //25
#define SPI0_GPIO_LED 23 //18

/* 
 * AuRPI3SPI0Map -- map the spi0 to kernel higher half address
 */
void AuRPI3SPI0Map() {
	spi0Base = (uint64_t*)AuMapMMIO(SPI0, 1);
	AuTextOut("[aurora]: RPI3 SPI0 mapped to %x address \r\n", spi0Base);
}

/*
 *AuRPI3SPI0Init -- initialize SPI0 of RPI3bp
 */
void AuRPI3SPI0Init() {
	/* configure pin 9,10,11,8 to alt function 0*/
	AuRPIGPIOSetFunction(9, 0b100);
	AuRPIGPIOSetFunction(10, 0b100);
	AuRPIGPIOSetFunction(11, 0b100);
	AuRPIGPIOSetFunction(8, 0b100);

	AuRPIGPIOSetFunction(SPI0_GPIO_DC, 1);
	AuRPIGPIOSetFunction(SPI0_GPIO_RST, 1);
	AuRPIGPIOSetFunction(SPI0_GPIO_LED, 1);

	*(volatile uint32_t*)((uint64_t)spi0Base + SPI0_CLK) = 64;
	dsb_ish();
	isb_flush();

	uint32_t cs = SPI_CS_CLEAR_RX | SPI_CS_CLEAR_TX | SPI_CS_CS0 | (1 << 7);
	AuTextOut("SPI CS Set to : %x \r\n", cs);
	
	*(volatile uint32_t*)((uint64_t)spi0Base + SPI0_CS) = cs;
	dsb_sy_barrier();
	isb_flush();
	for (int i = 0; i < 100000000; i++);
	;


	uint32_t csa = *(volatile uint32_t*)((uint64_t)spi0Base + SPI0_CS); 
	dsb_sy_barrier();
	isb_flush();
	uint32_t clk = *(volatile uint32_t*)((uint64_t)spi0Base + SPI0_CLK); 
	dsb_sy_barrier();
	isb_flush();

	AuTextOut("SPI CS After : %x \r\n", csa);
	AuTextOut("SPI0_CLK : %d \r\n", clk);

	AuTextOut("[aurora]:RPI3 SPI0 initialized \r\n");
}

#define SPI_MMIO_READ(off)   \
*(volatile uint32_t*)((uint64_t)spi0Base + off)

#define SPI_MMIO_WRITE(off) \
*(volatile uint32_t*)((uint64_t)spi0Base + off); \
dsb_ish(); \
isb_flush();


/*
 * AuRPISPITransfer -- transfer a single
 * 1 byte data
 * @param data -- data to transfer
 */
void AuRPISPITransfer(uint8_t data) {
	uint32_t cs = SPI_CS_TA | SPI_CS_CS0 | SPI_CS_CLEAR_TX | SPI_CS_CLEAR_RX;
	SPI_MMIO_WRITE(SPI0_CS, cs);
	//AuTextOut("SPITransferring data \r\n");

	while (!((SPI_MMIO_READ(SPI0_CS) & SPI_CS_TXD)));
	//AuTextOut("TXD is empty \r\n");

	SPI_MMIO_WRITE(SPI0_FIFO, data);

	//AuTextOut("FIFO Data written \r\n");

	while (!((SPI_MMIO_READ(SPI0_CS) & SPI_CS_DONE)));

	//AuTextOut("CS Done \r\n");

	while (!((SPI_MMIO_READ(SPI0_CS) & SPI_CS_RXD))) {
		SPI_MMIO_READ(SPI0_FIFO);
	}
	//cs = ~SPI_CS_TA | SPI_CS_CS0;
	SPI_MMIO_WRITE(SPI0_CS, SPI_CS_CS0);
}

/*
 * AuRPISPITransferBuffer -- transfer multiple 1 byte
 * aligned buffer data 
 * @param data -- pointer to buffer
 * @param len -- Length of total datas
 */
void AuRPISPITransferBuffer(const uint8_t* data, uint32_t len) {
	uint32_t cs = SPI_CS_TA | SPI_CS_CS0;

	SPI_MMIO_WRITE(SPI0_CS, cs);

	for (uint32_t i = 0; i < len; i++) {
		while (!(SPI_MMIO_READ(SPI0_CS) & SPI_CS_TXD));
		SPI_MMIO_WRITE(SPI0_FIFO, data[i]);
		while (SPI_MMIO_READ(SPI0_CS) & SPI_CS_RXD)
			SPI_MMIO_READ(SPI0_FIFO);
	}

	while (!(SPI_MMIO_READ(SPI0_CS) & SPI_CS_DONE));

	while (SPI_MMIO_READ(SPI0_CS) & SPI_CS_RXD)
		SPI_MMIO_READ(SPI0_FIFO);
	SPI_MMIO_WRITE(SPI0_CS, SPI_CS_CS0);
}

