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

#include <Drivers/uart.h>
#include <Mm/vmmngr.h>
#include <aucon.h>

uint64_t* uartMMIO;


static inline uint32_t uart_read_reg(uint32_t reg_offset) {
	return *(volatile uint32_t*)((uint64_t)uartMMIO + reg_offset);
}

static inline void uart_write_reg(uint32_t reg_offset, uint32_t value) {
	*(volatile uint32_t*)((uint64_t)uartMMIO + reg_offset) = value;
}

void UARTInitialize() {
	uartMMIO = (uint64_t*)AuMapMMIO(UART0_BASE, 1);
	AuTextOut("UART MMIO -> %x \n", uartMMIO);

	uart_write_reg(UART_CR, 0);
	uart_write_reg(UART_IBRD, 13);
	uart_write_reg(UART_FBRD, 1);

	uart_write_reg(UART_LCR_H, UART_LCR_H_WLEN_8BIT | UART_LCR_H_FEN);

	uart_write_reg(UART_CR, UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE);

	uart_write_reg(UART_ICR, 0xffff);

	//enable irq
	uart_write_reg(UART_IMSC, UART_IMSC_RXIM | UART_IMSC_RTIM);
}


void uartPutc(char c) {
	char* uart0 = (char*)uartMMIO;
	while ((*(uart0 + 0x18) & (1 << 5)));
	*uart0 = c;
}

/*
 * uartPuts --serial output interface
 * @param s -- String
 */
void uartPuts(const char* s) {
	while (*s)
		uartPutc(*s++);
}