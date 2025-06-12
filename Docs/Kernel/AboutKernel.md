## The XEKernel (_Xeneva Kernel_)

The _Xeneva Kernel_ is the main component of the entire operating system. It follows __Hybrid Kernel Design__, i.e a mixture of _Monolithic Kernel_ and _Microkernel_. The core functionalities (such as _device drivers, file system managements, IPC (Inter-Process Communication), scheduling, and low-level networking_) runs in kernel space. While some services (such as _Display Managements, Network Manager, Audio Services,..etc_) runs in user space.

## Core Functionalities
The core functionalities of the kernel are :
- Memory Management
- Device Driver Management
- File System Management
- IPCs
- Scheduling
- Network Management
- Process Management
- Kernel Audio Module
- System Call (_Service Calls_, in Xeneva's language)

## Xeneva Kernel Boot Protocol
The Xeneva boot protocol defines a simple and structured interface between the bootloader and the kernel, where the kernel expects a pointer to a KernelBootInfo structure to be passed as a parameter during boot. From very beginning the Kernel is designed to be booted from UEFI environment in both x86_64 and ARM64 architecture unless traditional non-UEFI boot method is required in ARM64. The structure contains essential system information such as memory layout, framebuffer configuration, hardware-specific data (_like Device Tree Blobs, pointer to ACPI tables_) and pointer to system files required for successfull kernel initialization. By using a well-defined structure, the protocol ensures that the kernel receives all critical information in a consistent format, allowing it to initialize subsystems across various platform.

### Technical Details of Xeneva Kernel Boot Protocol
Before bootloader calls the Kernel Entry Point, it must ensure that Kernel is properly loaded into memory and mapped to ***Kernel Virtual Base Address : which begins from 0xFFFFC00000000000***. Proper stack memory is also required size of 1MiB (0x100000). Kernel expects the KernelBootInfo structure as its first parameter of the Kernel Entry Point. For x86_64 MSVC calling convention, the first parameter goes to register ***rcx*** and for System V AMD64 ABI it goes to ***rdi***.  For ARM64 (_AAPCS64_), the first parameter goes to register ***x0***. This register holds the address of __KernelBootInfo__. Corrupted __KernelBootInfo__ or invalid address will cause the Kernel to behave improperly. 

### The KernelBootInfo structure:
```
/**
* Kernel Boot information structure passed by XNLDR
*/
#pragma pack (push,1)
typedef struct _KERNEL_BOOT_INFO_ {
	/* Boot type either UEFI_BOOT or BIOS_BOOT */
	int boot_type;

	void* allocated_stack;

	uint64_t reserved_mem_count;

	/* map -- UEFI memory map */
	void *map;

	/* descriptor_size -- UEFI memory map descriptor size */
	uint64_t descriptor_size;

	/* mem_map_size -- UEFI memory map size */
	uint64_t mem_map_size;

	/* graphics_framebuffer -- framebuffer address passed by XNLDR */
	uint32_t* graphics_framebuffer;

	/* fb_size -- framebuffer total size */
	size_t   fb_size;

	/* X_Resolution -- Total width of the entire display */
	uint16_t  X_Resolution;

	/* Y_Resolution -- Total height of the entire display */
	uint16_t   Y_Resolution;

	/* pixels_per_line -- scanline of the current display */
	uint16_t   pixels_per_line;

	/* redmask, greenmask, bluemask, resvmask -- color mask */
	uint32_t redmask;
	uint32_t greenmask;
	uint32_t bluemask;
	uint32_t resvmask;

	/* acpi_table_pointer -- acpi base memory map */
	void*    acpi_table_pointer;

	/* kernel_size -- total kernel image size */
	size_t   kernel_size;

	/* psf_font_data -- screen font address loaded
	by XNLDR to use for debugging purpose */
	uint8_t* psf_font_data;

	/* printf_gui -- formatted printing function pointer to use
	for debugging purpose provided by XNLDR */
	void(*printf_gui) (const char* text, ...);

	/* unused pointer entries, reserved for runtime drivers */
	uint8_t* driver_entry1;   
	uint8_t *driver_entry2;   
	uint8_t *driver_entry3;   
	uint8_t* driver_entry4;   
	uint8_t* driver_entry5;   
	uint8_t* driver_entry6;  

    /* multiprocessor initialisation code */
	void*    apcode;

    /* boot storage related informations */  
	uint32_t hid;
	uint32_t uid;
	uint32_t cid;
}KERNEL_BOOT_INFO, *PKERNEL_BOOT_INFO;
#pragma pack(pop)
```

- Here are the descriptions of each field in use

| Field Name | Type | Description |
|------------|------|-------------|
|```boot_type```   | ```int```  | Tells the Kernel from where it's booted, Available values are ```BOOT_UEFI_x64 1```, ```BOOT_UEFI_ARM64 2``` and ```BOOT_LITTLEBOOT_ARM64 3```
| ```allocated_stack``` | ```void*``` | The Bootloader allocates Physical Memory to load the kernel and it's required runtime data. This field is a stack pointer holding all allocated physical memory addresses, so that Kernel while using the _Kernel Physical Memory Manager_ does reserve these addresses, i.e store in a safe zone.
| ```reserve_mem_count``` | ```uint64_t``` | It holds the number of pages being allocated by Bootloader for Kernel code and runtime data, i.e the number of physical addresses stored in _allocated_stack_.
| ```map``` | ```void*``` | Pointer to UEFI memory map. _NOTE: Unused in boot_type ```BOOT_LITTLEBOOT_ARM64```_
| ```descriptor_size``` | ```uint64_t``` | UEFI memory map descriptor size. _NOTE: Unused in boot_type ```BOOT_LITTLEBOOT_ARM64```_
| ```mem_map_size``` | ```uint64_t``` | UEFI memory map size. _NOTE: Unused in boot_type ```BOOT_LITTLEBOOT_ARM64```_
| ```graphics_framebuffer``` | ```uint32_t*```| Pointer to Framebuffer address. If boot_type is ```BOOT_UEFI_x64``` or ```BOOT_UEFI_ARM64``` this field holds the address of GOP's (_Graphics Output Protocol_) framebuffer address. If boot_type is ```BOOT_LITTLEBOOT_ARM64``` this field might be unused, for this the Kernel utilize its Graphics Driver for framebuffer address and if LittleBoot implements Graphics Driver internally, this field is used.
| ```fb_size``` | ```size_t``` | The total size of framebuffer address, i.e total page occupied. _NOTE: in boot_type ```BOOT_LITTLEBOOT_ARM64``` this might be used if LittleBoot implements Graphics Driver, else unused.
| ```X_Resolution``` | ```uint16_t``` | Size of screen in width.
| ```Y_Resolution``` | ```uint16_t``` | Size of screen in height.
| ```pixels_per_line```| ```uint16_t```| The number of pixels displayed horizontally in a single row of the screen
| ```redmask``` | ```uint32_t``` | Tells which bits in 32-bit pixels correspond to red.
| ```greenmask``` | ```uint32_t```| Tells which bits in 32-bit pixels correspond to green.
| ```bluemask``` | ```uint32_t``` | Tells which bits in 32-bit pixels correspond to blue.
| ```resvmask``` | ```uint32_t```| Telss which bits in 32-bit pixels are reserved.
|```acpi_table_pointer```| ```void*```| Pointer to ACPI tables _xsdp_. _NOTE: Unused in boot_type ```BOOT_LITTLEBOOT_ARM64```_
|```kernel_size``` | `size_t` | Total size of Kernel in bytes
| ```psf_font_data``` | `uint8_t*` | Pointer to PSF font data start. Used only on boot_type _```BOOT_UEFI_x64```_
| ```printf_gui``` | _function pointer_ | Formatted text printing function implemented in Bootloader ,used by kernel during early initialization phase. _NOTE: Unused in boot_type ```BOOT_LITTLEBOOT_ARM64```_
|```driver_entry1 - driver_entry6``` | ```uint8_t*``` | Slots available for using as pointer to Kernel Boot Time drivers. During Kernel early initialization phase, it cannot load the boot time drivers from boot storage because boot storage driver itself is a boot time driver. The bootloader is responsible for loading the boot time drivers and mapping it to Kernel virtual address space. ***Boot time drivers are mapped from based address: 0xFFFFC00000400000***. _NOTE: This field can also be used for _initrd_ like image. If boot_type is ```BOOT_LITTLEBOOT_ARM64``` the driver_entry1 is used as pointer to LittleBootInfo structure.
| ```ap_code``` | ```void*``` | Pointer to AP (_Application Processors_) initialization code.
| ```hid``` | ```uint32_t``` | _Unused_
| ```uid``` | ```uint32_t``` | _Unused_
| ```cid``` | ```uint32_t``` | _Unused_







