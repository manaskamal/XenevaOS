/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#include <aurora.h>
#include <stdint.h>
#include <aucon.h>
#include <dtb.h>
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <Mm/kmalloc.h>
#include <Mm/shm.h>
#include <_null.h>
#include <Hal/basicacpi.h>
#include <Drivers/uart.h>
#include <list.h>
#include <Fs/vfs.h>
#include <Fs/initrd.h>
#include <loader.h>
#include <audrv.h>
//#include <Hal/sched.h>
#include <Fs/tty.h>
#include <ftmngr.h>
#include <Ipc/postbox.h>

// RISC-V specific headers
#include "Hal/riscv64_hal.h"
#include "Hal/riscv64_lowlevel.h"

extern "C" int _fltused = 1;
bool _littleboot_used = false;

// QEMU RISC-V Virt UART
#define UART_BASE 0x10000000
#define UART_THR 0x00
#define UART_LSR 0x05

void AuUartPutc(char c) {
    volatile uint8_t* uart = (volatile uint8_t*)UART_BASE;
    while ((uart[UART_LSR] & 0x20) == 0);
    uart[UART_THR] = c;
}

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

KERNEL_BOOT_INFO* bootinfo;

/*
 * AuGetBootInfoStruc -- return kernel boot information
 */
extern "C" KERNEL_BOOT_INFO* AuGetBootInfoStruc() {
	return bootinfo;
}


extern void debugLIBOn();

/*
 * _AuMain -- the main entry point for kernel
 * @param info -- Kernel Boot information passed
 * by bootloader
 */
extern "C" void _AuMain(KERNEL_BOOT_INFO* info) {
	bootinfo = info;
    
	AuConsoleInitialize(info, true);
    AuDeviceTreeInitialize(info);

    // Architecture specific CPU Init
	RISCV64CpuInitialize();
    
	AuTextOut("[aurora]: CPU initialized \r\n");
    
    
	AuPmmngrInitialize(info);
    // VMM Init might need arch specific paging code in Mm/
	AuVmmngrInitialize();
    
	AuTextOut("[aurora]: virtual memory manager initialized \r\n");
	AuHeapInitialize();
    
	AuDeviceTreeMapMMIO();
    AuRISCV64BoardInitialize();

	RISCV64CPUPostInitialize(info);

	AuVFSInitialise();
	AuInitrdInitialize(info);
	AuConsolePostInitialise(info);

	/* initialize the tty service */
	AuTTYInitialise();

	/* initialize the shared memory manager*/
	AuInitialiseSHMMan();

	/* initialize our basic requirement */
    // VirtIO init if supported
	// AuVirtIOInputInitialize();

	/* required virtio-mouse and keyboard */
	//Here goes board pre driver initialize
	AuDrvMngrInitialize(info);

	FontManagerInitialise();
	
	AuInitialiseLoader();

	/* initialize the deodhai's communication protocol */
	AuIPCPostBoxInitialise();

	AuTextOut("[aurora]: starting xeneva (RISC-V 64) please wait...\r\n");

	/* clear out the lower half memory */
	AuVmmngrBootFree();

	AuTextOut("[aurora]: address space lower half cleared successfully \r\n");

	// AuSchedulerInitialize(); // Arch specific scheduler context switch?

	// test
    AuTextOut("System Halted. \n");
	while (1) {
		//UARTDebugOut("Printing \n");
	}
}
