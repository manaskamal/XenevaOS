/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#include "uart0.h"
#include "clib.h"

void XEUartInitialize() {
    volatile uint8_t* uart = (volatile uint8_t*)UART_BASE;
    // Basic Initialization for NS16550A
    // 1. Disable Interrupts
    uart[UART_IER] = 0x00;
    
    // 2. Enable FIFO
    uart[UART_FCR] = 0x01; // Enable FIFO
    
    // 3. Set standard 8N1
    uart[UART_LCR] = 0x03; // 8 bits, 1 stop, no parity
}

/*
 * XEUartPutc -- put a character to UART
 * @param c -- Character to put
 */
void XEUartPutc(char c) {
    volatile uint8_t* uart = (volatile uint8_t*)UART_BASE;
    // Wait for THRE (Transmitter Holding Register Empty) - Bit 5 in LSR
    while ((uart[UART_LSR] & 0x20) == 0);
    
    uart[UART_THR] = c;
}

/*
 * XEUartPutString -- print a string
 * to UART
 * @param s -- String to print
 */
void XEUartPutString(const char* s) {
	while (*s)
		XEUartPutc(*s++);
}

extern "C" void store_a0_a7(uint64_t * buffer);

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
	store_a0_a7(buffer);

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
