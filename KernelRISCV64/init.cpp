/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* Copyright (c) 2026, Aryan Dadwal
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
#include <Hal/riscv64_sched.h>
#include <Fs/tty.h>
#include <ftmngr.h>
#include <Ipc/postbox.h>
#include <process.h>

// RISC-V specific headers
#include "Hal/riscv64_hal.h"
#include "Hal/riscv64_lowlevel.h"

extern "C" void AuVirtIOGPUMMIOAttach();
extern "C" void arch_enter_user(uint64_t user_stack, uint64_t user_entry);

extern "C" int _fltused = 1;
bool _littleboot_used = false;

// QEMU RISC-V Virt UART
#define UART_BASE_PHYS 0x10000000
#define UART_BASE_VIRT 0xFFFFFFD010000000ULL
#define UART_THR 0x00
#define UART_LSR 0x05

void AuUartPutc(char c) {
    // If VMM is initialized, use Virtual Address. How do we know?
    // We can check if SATP is set to the kernel table.
    // For now, AuConsoleEarlyEnable(false) switches to a method that uses AuTextOut.
    // AuTextOut calls this function directly? No, AuTextOut in init uses AuUartPutString.
    // We need to switch this pointer after VMM init.
    
    // Quick hack: Try Virtual, if fault, trap handler (oh wait we can't depend on trap yet).
    // Better: Helper boolean or Check SATP.
    // Or just use lower half Identity (which we kept in sync for boot info).
    // The bootloader Identity map is cloned.
    // So 0x10000000 IS valid.
    // So we don't strictly need to change this if Identity Map persists.
    // But `AuVmmngrBootFree` unmaps lower half.
    // So we MUST use virtual address after that.
    
    static bool use_virt = false;
    // We can export a function to flip this.
    // Or check `sp`?
    
    volatile uint8_t* uart = (volatile uint8_t*)UART_BASE_PHYS;
    // Check if we are in High Half (PC > 0xFFFFFFFF00000000)
    // If yes, we probably have VMM.
    // But even with VMM, Identity might exist.
    // Once we unmap ID, we must use VIRT.
    // Let's assume VIRT if high bit set?
    // Let's rely on a global flag.
    
    extern bool _uart_use_virt; // Define in .cpp body
    if (_uart_use_virt) uart = (volatile uint8_t*)UART_BASE_VIRT;
    
    while ((uart[UART_LSR] & 0x20) == 0);
    uart[UART_THR] = c;
}

bool _uart_use_virt = false;

extern "C" void AuHalVirtUartEnable() {
    _uart_use_virt = true;
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
extern "C" void AuSwitchStackAndJump(uint64_t new_sp, void (*entry)(KERNEL_BOOT_INFO*), KERNEL_BOOT_INFO* arg);

/*
 * AuKernelMain -- High Half Entry Point
 */
void AuKernelMain(KERNEL_BOOT_INFO* info) {
    AuTextOut("[aurora]: Stack switched to High Half (AuKernelMain) \r\n");
    /* After VMM switch, the PMM bitmap pointer is still physical.
     * Convert it to HHDM virtual so AuPmmngrAlloc works correctly.
     * AuVmmngrBootFree() is still deferred (GOT/reloc issue). */
    extern void AuPmmngrMoveHigher();
    AuPmmngrMoveHigher();
    
	AuTextOut("[aurora]: virtual memory manager initialized \r\n");
	AuHeapInitialize();
    
	AuDeviceTreeMapMMIO();
    AuRISCV64BoardInitialize();

	RISCV64CPUPostInitialize(info);
    
    // Enable Global Interrupts (SIE bit in sstatus)
    extern void enable_irq();
    enable_irq();
    AuTextOut("[aurora]: Global Interrupts Enabled \r\n");

	AuVFSInitialise();
	AuInitrdInitialize(info);
	AuConsolePostInitialise(info);

	/* initialize the tty service */
	AuTTYInitialise();

	/* initialize the shared memory manager*/
	AuInitialiseSHMMan();

	/* initialize our basic requirement */
    // VirtIO init if supported
	extern void AuVirtIOInputInitialize();
	AuVirtIOInputInitialize();

	/* required virtio-mouse and keyboard */
	//Here goes board pre driver initialize
	AuDrvMngrInitialize(info);
	
	/* Initialize Explicit Boot Drivers natively compiled into kernel */
	AuVirtIOGPUMMIOAttach();

	// FontManagerInitialise();
    // AuTextOut("[aurora]: Skipped FontManager (Dummy InitRD) \r\n");
	// FontManagerInitialise();
    // AuTextOut("[aurora]: Skipped FontManager (Dummy InitRD) \r\n");
    FontManagerInitialise();
    AuTextOut("[aurora]: FontManager initialized \r\n");

    AuInitialiseLoader();

	/* initialize the deodhai's communication protocol */
	AuIPCPostBoxInitialise();

	AuTextOut("[aurora]: starting xeneva (RISC-V 64) please wait...\r\n");

	AuSchedulerInitialise();
	AuTextOut("[aurora]: Scheduler initialized\r\n");

	/* Launch the real init process from InitRD or Buffer */
	AuProcess* proc = AuCreateProcessSlot(0, (char*)"init");
	if (proc) {
		AuTextOut("[init]: Process slot created, cr3=%x\r\n", (uint64_t)proc->cr3);
		int ret = -1;
		if (info->driver_entry2 != 0) {
			AuTextOut("[init]: Loading init.exe directly from Bootloader RAM buffer...\r\n");
			ret = AuLoadExecFromBuffer(proc, (void*)info->driver_entry2, (char*)"init.exe", 0, NULL);
		} else {
			ret = AuLoadExecToProcess(proc, (char*)"/init.exe", 0, NULL);
		}
		
		if (ret == 0) {
			AuTextOut("[init]: /init.exe loaded successfully.\r\n");
		} else {
			AuTextOut("[aurora]: ERROR - Failed to load /init.exe! Check InitRD or Bootloader.\r\n");
		}
	} else {
		AuTextOut("[aurora]: ERROR - Failed to create process slot!\r\n");
	}

	/* NOTE: AuVmmngrBootFree() is deferred — the RISC-V kernel's GOT/reloc entries
	 * reference physical load addresses, which need the identity map to be present.
	 * This is a known limitation of the fake-PE build approach.
	 * TODO: Fix the linker/relocation pipeline to use only virtual addresses.
	 */
	// AuVmmngrBootFree();

	AuSchedulerStart();
	while (1) {
	}
}

/*
 * _AuMain -- the main entry point for kernel
 * @param info -- Kernel Boot information passed
 * by bootloader
 */
extern "C" void _AuMain(KERNEL_BOOT_INFO* info) {
    // Preserve BootInfo before PMM/VMM potentially trash EfiLoaderData
    static KERNEL_BOOT_INFO _bootinfo_storage;
    for (int i=0; i<sizeof(KERNEL_BOOT_INFO); i++) {
        ((uint8_t*)&_bootinfo_storage)[i] = ((uint8_t*)info)[i];
    }
    info = &_bootinfo_storage;
	bootinfo = info;
    
	AuConsoleInitialize(info, true);
    AuDeviceTreeInitialize(info);

    // Architecture specific CPU Init
	RISCV64CpuInitialize();
    
	AuTextOut("[aurora]: CPU initialized \r\n");
    
    AuPmmngrInitialize(info);
    // VMM Init might need arch specific paging code in Mm/
	AuVmmngrInitialize(info);

    // Switch to Kernel Printing (UART Fallback)
    // The bootloader print function is no longer mapped!
    AuConsoleEarlyEnable(false);
    
    // Enable Virtual UART
    void AuHalVirtUartEnable();
    AuHalVirtUartEnable();

    AuTextOut("[aurora]: VMM Initialized. Switching Stack...\n");
    
    // Switch to High Half Stack and Jump to KernelMain
    // StackBase = 0xFFFFFFC000F00000
    // StackSize = 16 * 4096 = 65536 (0x10000)
    // Top = 0xFFFFFFC000F10000
    AuSwitchStackAndJump(0xFFFFFFC000F10000, AuKernelMain, info);
}
