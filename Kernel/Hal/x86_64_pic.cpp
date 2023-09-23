/**
* BSD 2-Clause License
*
* Copyright (c) 2022, Manas Kamal Choudhury
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

#include <Hal\x86_64_pic.h>
#include <aucon.h>
#include <Hal\x86_64_hal.h>

void AuPICInterruptEOI(unsigned int intno) {
	if (intno > 16)
		return;

	if (intno >= 8)
		x64_outportb(PIC2_COMMAND, PIC_EOI);

	x64_outportb(PIC1_COMMAND, PIC_EOI);
}

static uint64_t pic_counter = 0;

void AuPITHandler(size_t v, void* param) {
	pic_counter++;
	AuPICInterruptEOI(0);
}

void AuPICSetMask(uint8_t irq) {
	uint16_t port;
	uint8_t value;

	if (irq < 8)
		port = PIC1_DATA;
	else{
		port = PIC2_DATA;
		irq -= 8;
	}

	value = x64_inportb(port) | (1 << irq);
	x64_outportb(port, value);
}

void AuPICClearMask(uint8_t irq) {
	uint16_t port;
	uint8_t value;

	if (irq < 8)
		port = PIC1_DATA;
	else {
		port = PIC2_DATA;
		irq -= 8;
	}

	value = x64_inportb(port) & ~(1 << irq);
	x64_outportb(port, value);
}

void AuPITSleepMS(uint32_t ms) {
	static int ticks = ms + pic_counter;
	while (ticks > pic_counter)
		;
}

void AuInitialisePIC() {
	AuTextOut("iNITIALISING PIC \n");
	uint8_t base0 = 0x20;
	uint8_t base1 = 0x28;

	unsigned char a1, a2;
	a1 = x64_inportb(PIC1_DATA);
	a2 = x64_inportb(PIC2_DATA);

	x64_outportb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	for (int i = 0; i < 1000; i++)
		;
	x64_outportb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	for (int i = 0; i < 1000; i++)
		;
	x64_outportb(PIC1_DATA, base0);
	for (int i = 0; i < 1000; i++)
		;
	x64_outportb(PIC2_DATA, base1);
	for (int i = 0; i < 1000; i++)
		;
	x64_outportb(PIC1_DATA, 4);
	for (int i = 0; i < 1000; i++)
		;
	x64_outportb(PIC2_DATA, 2);
	for (int i = 0; i < 1000; i++)
		;
	x64_outportb(PIC1_DATA, ICW4_8086);
	for (int i = 0; i < 1000; i++)
		;
	x64_outportb(PIC2_DATA, ICW4_8086);
	for (int i = 0; i < 1000; i++)
		;
	x64_outportb(PIC1_DATA, a1);
	x64_outportb(PIC2_DATA, a2);

	unsigned int divisor = 1193181 / 100;
	x64_outportb(0x43, 0x00 | 0x06 | 0x30 | 0x00);
	x64_outportb(0x40, divisor);
	x64_outportb(0x40, divisor >> 8);
	setvect(32 + 0, AuPITHandler);
	AuTextOut("PIC/PIT initialised \n");
	pic_counter = 0;
	AuPICClearMask(0);
}