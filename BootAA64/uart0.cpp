/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2025, Manas Kamal Choudhury
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

#include "uart0.h"
#include "rpi3b/mbox.h"
#include "clib.h"

void XEUartInitialize() {
#ifdef __TARGET_BOARD_RPI3__
	RPI3BUartInit();
#endif
}

/*
 * LBUartPutc -- put a character to UART
 * @param c -- Character to put
 */
void XEUartPutc(char c) {
	char* uart0 = (char*)UART_BASE;
	while ((*(uart0 + 0x18) & (1 << 5)));
	*uart0 = c;
}

/*
 * LBUartPutString -- print a string
 * to UART
 * @param s -- String to print
 */
void XEUartPutString(const char* s) {
	while (*s)
		XEUartPutc(*s++);
}

extern "C" void store_x0_x7(uint64_t * buffer);

/*
 * XEUARTPrint -- print formated text using graphics
 * @param format -- formated string
 */
void XEUARTPrint(const char* format, ...) {
	/* Hacky, printf implementation, it doesn't automatically
	 * store the registered in stack, rather in a shadow area
	 * and i don't know how to access those
	 */
	uint64_t buffer[7];
	store_x0_x7(buffer);

	va_list args = (va_list)buffer;
	//va_start(args, format);
	while (*format != '\0') {
		if (*format == '%') {
			++format;
			if (*format == 'd') {
				size_t width = 0;
				if (format[1] == '.') {
					for (size_t i = 2; format[i] >= '0' && format[i] <= '9'; ++i)
					{
						width *= 10;
						width += format[i] - '0';
					}
				}
				size_t i = va_arg(args, size_t);
				char buffer[sizeof(size_t) * 8 + 1];
				sztoa(i, buffer, 10);
				size_t len = strlen(buffer);
				while (len++ < width)
					XEUartPutString("0");
				XEUartPutString(buffer);
			}
			else if (*format == 'c') {

				char c = va_arg(args, char);
				XEUartPutc(c);
			}
			else if (*format == 'x') {
				size_t x = va_arg(args, size_t);
				char buffer[sizeof(size_t) * 8 + 1];
				sztoa(x, buffer, 16);
				XEUartPutString(buffer);
			}
			else if (*format == 's') {
				char* x = va_arg(args, char*);
				XEUartPutString(x);
			}
			else if (*format == 'S') {
				char* x = va_arg(args, char*);
				XEUartPutString(x);
			}
			else if (*format == '%') {
				XEUartPutString(".");
			}
			else {
				char buf[3];
				buf[0] = '%'; buf[1] = *format; buf[2] = '\0';
				XEUartPutString(buf);
			}
		}
		else
		{
			char buf[2];
			buf[0] = *format; buf[1] = '\0';
			XEUartPutString(buf);
		}
		++format;
	}
	va_end(args);
}


