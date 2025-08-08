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
#include <loader.h>
#include <Drivers/uart.h>
#include <Hal/AA64/qemu.h>
#include <list.h>
#include <Fs/vfs.h>
#include <Fs/initrd.h>
#include <Drivers/virtiogpu.h>
#include <audrv.h>
#include <Hal/AA64/sched.h>


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

int i_ = 1;

extern void sub_rsp();

void AuEntryTest(uint64_t test) {
	//aa64_utest();
	int c = 10;
	//enable_irqs();
	while (1) {
		//enable_irqs();
		/*if ((c % 2) == 0)
			UARTDebugOut("2\n");
		for (int i = 0; i < 10000000; i++)
			;*/
		c++;
	}
}

void AuEntryTest2(uint64_t test) {
	int d = 10;
	//enable_irqs();
	AA64Thread* thr = AuThreadFindByIDBlockList(1);
	if (thr) {
		UARTDebugOut("Unblocking thr : %s \n", thr->name);
		AuUnblockThread(thr);
	}
	while (1) {
		/*enable_irqs();
		if ((d % 2) != 0)
			UARTDebugOut("3 \n");*/
		d++;
	}
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

	/* required virtio-mouse and keyboard */
	//Here goes board pre driver initialize
	AuDrvMngrInitialize(info);

	AuInitialiseLoader();
	
	/* clear out the lower half memory */
	AuVmmngrBootFree();

	AuSchedulerInitialize();
	UARTDebugOut("SIZEOF (AuUserEntry) - %d \n", sizeof(AuUserEntry));
	AuProcess* proc = AuCreateProcessSlot(0, "exec");
	AuLoadExecToProcess(proc, "/test.exe", 0, NULL);

	AA64Thread* thr = AuCreateKthread(AuEntryTest2, AuCreateVirtualAddressSpace(), "Test");
	AA64Thread* thr2 = AuCreateKthread(AuEntryTest, AuCreateVirtualAddressSpace(), "Test2User");
	//UARTDebugOut("Offset of ThreadType - %d \n", (&thr2->threadType - thr));

	UARTDebugOut("Kernel Lower half is cleared \n");
	
	AuSchedulerStart();
	while (1) {
		//UARTDebugOut("Printing \n");
	}
}
