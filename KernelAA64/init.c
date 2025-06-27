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
#include <list.h>
#include <Fs/vfs.h>
#include <Fs/initrd.h>
#include <Drivers/virtiogpu.h>
#include <audrv.h>


extern int _fltused = 1;
static bool _littleboot_used;


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
 * AuLittleBootUsed -- check if little boot protocol
 * is used
 */
bool AuLittleBootUsed() {
    return _littleboot_used;
}

/*
 * _AuMain -- the main entry point for kernel
 * @param info -- Kernel Boot information passed
 * by bootloader
 */
void _AuMain(KERNEL_BOOT_INFO* info) {
    _littleboot_used = false;
    if (info->boot_type == BOOT_LITTLEBOOT_ARM64) {
        AuUartPutString("[aurora]:Kernel is booted using LittleBoot ARM64 \n");
        _littleboot_used = true;
    }
	AuConsoleInitialize(info, true);
    AuDeviceTreeInitialize(info);
	AA64CpuInitialize();
	AuPmmngrInitialize(info);
	AuVmmngrInitialize();
	AuHeapInitialize();
	AuDeviceTreeMapMMIO();
	AuConsolePostInitialise(info);
	AA64CPUPostInitialize(info);
	AuVFSInitialise();
	AuInitrdInitialize(info);
	AuDrvMngrInitialize(info);
	/* need to initialize basic drivers here*/
	/* scheduler initialize*/
	/* scheduler start*/
	while (1) {
	}
}
