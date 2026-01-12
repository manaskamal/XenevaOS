/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#ifndef __UART0_H__
#define __UART0_H__

#include "xnldr.h"

// QEMU RISC-V Virt UART Base (NS16550A)
#define UART_BASE 0x10000000

// NS16550A Register Offsets
#define UART_THR 0x00 // Transmitter Holding Register (Write)
#define UART_RBR 0x00 // Receiver Buffer Register (Read)
#define UART_IER 0x01 // Interrupt Enable Register
#define UART_FCR 0x02 // FIFO Control Register
#define UART_ISR 0x02 // Interrupt Status Register
#define UART_LCR 0x03 // Line Control Register
#define UART_MCR 0x04 // Modem Control Register
#define UART_LSR 0x05 // Line Status Register
#define UART_MSR 0x06 // Modem Status Register
#define UART_SCR 0x07 // Scratch Register

extern void XEUartInitialize();

/*
 * XEUARTPrint -- print formated text using graphics
 * @param format -- formated string
 */
extern void XEUARTPrint(const char* format, ...);


#endif