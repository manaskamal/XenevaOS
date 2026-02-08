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
    // AuVmmngrBootFree(); // Disable for now to keep info/map accessible
    
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
	// AuVirtIOInputInitialize();

	/* required virtio-mouse and keyboard */
	//Here goes board pre driver initialize
	AuDrvMngrInitialize(info);

	// FontManagerInitialise();
    // AuTextOut("[aurora]: Skipped FontManager (Dummy InitRD) \r\n");
	// FontManagerInitialise();
    // AuTextOut("[aurora]: Skipped FontManager (Dummy InitRD) \r\n");
    FontManagerInitialise();
    AuTextOut("[aurora]: FontManager initialized \r\n");

    // --- User Mode Test ---
    AuTextOut("[aurora]: Preparing User Mode Test...\r\n");
    
    // 1. Allocate Code Page
    uint64_t phys_code = (uint64_t)AuPmmngrAlloc();
    if (!phys_code) {
        AuTextOut("[aurora]: Panic! Failed to allocate physical, code page\r\n");
        while(1);
    }
    uint64_t* virt_code = (uint64_t*)P2V(phys_code);
    memset(virt_code, 0, 4096);
    
    // 2. Allocate Stack Page
    uint64_t phys_stack = (uint64_t)AuPmmngrAlloc();
    if (!phys_stack) {
        AuTextOut("[aurora]: Panic! Failed to allocate physical, stack page\r\n");
        while(1);
    }
    uint64_t* virt_stack = (uint64_t*)P2V(phys_stack);
    memset(virt_stack, 0, 4096);
    
    // 3. Map to User Space
    // User Code at 0x1000000
    // User Stack at 0x2000000
    AuMapPage(phys_code, 0x1000000, PTE_READ | PTE_WRITE | PTE_EXEC | PTE_USER);
    AuMapPage(phys_stack, 0x2000000, PTE_READ | PTE_WRITE | PTE_USER); // Stack identity mapped for simplicity in this test?
    // Let's map stack to 0x0000000080000000 (standard user top) or just 0x2000000
    // Using 0x2000000 for safety.
    AuMapPage(phys_stack, 0x2000000, PTE_READ | PTE_WRITE | PTE_USER); 
    
    AuTextOut("[aurora]: Mapped User Code (0x1000000) and Stack (0x2000000)\r\n");
    
    // 4. Write Payload to Code Page
    // li a7, 1 (Syscall 1)
    // ecall
    // j .
    
    // li a7, 1 -> addi a7, zero, 1 -> 0x00100893
    // ecall    -> 0x00000073
    // j .      -> 0x0000006f
    
    uint32_t* code = (uint32_t*)virt_code;
    code[0] = 0x00100893; 
    code[1] = 0x00000073; 
    code[2] = 0x0000006f; 
    
    // Fence to ensure I-Cache sees the new instructions
    sfence_vma();
    
    AuTextOut("[aurora]: Entering User Mode...\r\n");
    
    // Stack grows down, so point to End of page (0x2000000 + 4096)
    arch_enter_user(0x2000000 + 4096, 0x1000000);
    
    // Should not return here immediateley unless trap returns to this context (which it won't, it returns to user)
    // But if our trap handler advances SEPC and returns, the USER code will loop.
    // If we want to return TO KERNEL after test, we need a special syscall to 'exit'.
    // For now, let it loop or trap.
    
	AuInitialiseLoader();

	/* initialize the deodhai's communication protocol */
	AuIPCPostBoxInitialise();

	AuTextOut("[aurora]: starting xeneva (RISC-V 64) please wait...\r\n");

	/* clear out the lower half memory */
	// AuVmmngrBootFree();
    AuTextOut("[aurora]: Skipped freeing lower half (Stack is there)\r\n");

	AuTextOut("[aurora]: address space lower half cleared successfully \r\n");

	// AuSchedulerInitialize(); // Arch specific scheduler context switch?

	// test
    AuTextOut("System Halted. \n");
	while (1) {
		//UARTDebugOut("Printing \n");
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
