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
#include <va_list.h>
#include <stdarg.h>
#include <string.h>

uint64_t* uartMMIO;
bool _uart_mapped = false;

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
	_uart_mapped = true;
}


void uartPutc(char c) {
	uint64_t* mmioBase = 0;
	if (_uart_mapped)
		mmioBase = uartMMIO;
	else
		mmioBase = UART0_BASE;
	char* uart0 = (char*)mmioBase;
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

/*
 * UARTDebugOut -- standard text printing function
 * for early kernel using UART
 * @param text -- text to output
 */
void UARTDebugOut_Call(const char* format, void* reg_save_area, void* entry_sp) {

	va_list args = ((va_list)reg_save_area + 8);
	while (*format)
	{
		if (*format == '%')
		{
			++format;
			if (*format == 'd')
			{
				size_t width = 0;
				if (format[1] == '.')
				{
					for (size_t i = 2; format[i] >= '0' && format[i] <= '9'; ++i)
					{
						width *= 10;
						width += format[i] - '0';
					}
				}
				size_t i = va_arg(args, size_t);
				char buffer[sizeof(size_t) * 8 + 1];
				//	size_t len
				if ((int)i < 0) {
					uartPuts("-");
					i = ((int)i * -1);
					sztoa(i, buffer, 10);
				}
				else {
					sztoa(i, buffer, 10);
					size_t len = strlen(buffer);
				}
				/*	while (len++ < width)
				puts("0");*/
				uartPuts(buffer);
			}
			else if (*format == 'c')
			{
				char c = va_arg(args, char);
				//char buffer[sizeof(size_t) * 8 + 1];
				//sztoa(c, buffer, 10);
				//puts(buffer);
				uartPutc(c);
			}
			else if (*format == 'x')
			{
				size_t x = va_arg(args, size_t);
				char buffer[sizeof(size_t) * 8 + 1];
				sztoa(x, buffer, 16);
				//puts("0x");
				uartPuts(buffer);
			}
			else if (*format == 's')
			{
				char* x = va_arg(args, char*);
				uartPuts(x);
			}
			else if (*format == 'f')
			{
				double x = va_arg(args, double);
				uartPuts(ftoa(x, 2));
			}
			else if (*format == '%')
			{
				uartPuts(".");
			}
			else
			{
				char buf[3];
				buf[0] = '%'; buf[1] = *format; buf[2] = '\0';
				uartPuts(buf);
			}
		}
		else
		{
			char buf[2];
			buf[0] = *format; buf[1] = '\0';
			uartPuts(buf);
		}
		++format;
	}
	va_end(args);
}