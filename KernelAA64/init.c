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

#include <aurora.h>
#include <stdint.h>
#include <aucon.h>
#include <Hal/AA64/aa64cpu.h>
#include <dtb.h>
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <Mm/kmalloc.h>
#include <_null.h>
#include <Hal/AA64/gic.h>
#include <Hal/basicacpi.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Drivers/uart.h>
#include <Hal/AA64/qemu.h>


extern int _fltused = 1;

/*
 * LBUartPutc -- put a character to UART
 * @param c -- Character to put
 */
void AuUartPutc(char c) {
	char* uart0 = (char*)UART0;
	while ((*(uart0 + 0x18) & (1 << 5)));
	*uart0 = c;
}

/*
 * LBUartPutString -- print a string
 * to UART
 * @param s -- String to print
 */
void AuUartPutString(const char* s) {
	while (*s)
		AuUartPutc(*s++);
}

/*
 * LBUartPrintInt -- prints integer
 * through UART
 * @param value -- value to print
 */
void LBUartPrintInt(uint64_t value) {
    char buffer[12];
    int i = 0;
    bool is_negative = false;
    if (value == 0) {
        AuUartPutc('0');
        return;
    }

    if (value < 0) {
        is_negative = true;
        value = -value;
    }

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    if (is_negative)
        buffer[i++] = '-';

    while (i--)
        AuUartPutc(buffer[i]);

    AuUartPutc('\n');
}

/*
 * LBUartPrintHex -- prints hexadecimal value
 * using serial UART
 * @param val -- value to print
 */
void LBUartPrintHex(uint64_t val) {
    AuUartPutc('0');
    AuUartPutc('x');
    for (int i = 60; i >= 0; i -= 4) {
        uint8_t nibble = (val >> i) & 0xF;
        char hex = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
        AuUartPutc(hex);
    }
    AuUartPutc('\n');
}


void _AuMain(KERNEL_BOOT_INFO* info) {
	AuUartPutString("Hey inside from kernel !! \n");
    LBUartPrintHex(info);
    LBUartPrintInt(info->boot_type);
	if (info->boot_type == BOOT_LITTLEBOOT_ARM64)
		AuUartPutString("Kernel is booted using LittleBoot ARM64 \n");
	for (;;);
	AuConsoleInitialize(info, true);
	AuDeviceTreeInitialize(info->apcode);
	AA64CpuInitialize();
	AuPmmngrInitialize(info);
	AuVmmngrInitialize();
	AuHeapInitialize();
	AuConsolePostInitialise(info);
	AA64CPUPostInitialize(info);


	while (1) {
	}
}
