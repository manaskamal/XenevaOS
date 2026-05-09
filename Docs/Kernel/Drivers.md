# Drivers in Xeneva

Drivers are specialized software components that enable an Operating System to communicate with hardware devices. In Xeneva, kernel mode drivers helps the entire OS to communicate with hardware devices. There are two types of drivers : 
- __Kernel mode drivers__
- __User mode drivers__

Current versions of Xeneva supports only _Kernel mode drivers_.

## Boot Time Drivers
Boot time drivers are the drivers that gets loaded prior to the Kernel for basic runtime of Kernel. Boot time drivers are loaded by XNLDR which loads all necessary boot drivers and maps it to _Kernel Space_ (_Higher half memory_). Some example of boot time drivers are all _Storage Drivers_ , _File System drivers_ and ACPI driver. Storage Drivers includes (_NVMe driver, AHCI/SATA Controller driver, IDE drivers and USB Mass Storage driver_). <br>
Xeneva Kernel depends on _Storage Drivers_ and _File System_ drivers to further run the system. The drivers are loaded and mapped to Kernel Space by XNLDR and the address is passed to the kernel through _Boot Information_. Xeneva Kernel includes special functions that handles boot time drivers seperately from runtime drivers. The function _AuBootDriverInitialise_ starts loading of all boot drivers. Linking of boot drivers to kernel functions is done internally in the kernel. 

## Runtime Drivers
Once boot drivers like _Storage Drivers_ are loaded and initialized successfully, kernel uses those drivers to further load and start runtime drivers. Runtime Drivers includes the _Network Drivers_, _Sound Drivers_, _Graphics Drivers_, ..etc. _Network Drivers includes Ethernet Drivers_. Sound Drivers includes _High Definition Audio Driver_ ..etc. Runtime drivers are loaded and linked during kernel runtime. <br>
Firstly, Kernel open and read the driver configuration file from the root file system. The driver configuration file contains the path of drivers along with its PCIe class-subclass-progif values. The kernel reads each entry from the configuration file and load all PCIe drivers by matching class-subclass-progif values. 

## Initialization
Though Boot time drivers and Runtime drivers are seperately handled but the protocol they follows are same. The drivers must meet some requirements that the Kernel expects to run properly. Some kernel function calls are mostly required.
- ***AuPCIEScanClass*** : This function returns the bus, device and function number by its classCode and subClassCode from PCIe ( _NOTE: Xeneva only supports PCIe_)
- ***AuPCIERead/AuPCIEWrite*** : Reads from and Writes to PCIe device
- ***AuPCIEAllocMSI*** : Allocates MSI/MSI-X signal for provided device
- ***setvect*** : Register an interrupt vector for provided device
- ***struct AuDevice*** : The Device structure that holds all device related informations
- ***AuRegisterDevice*** : Registers the device to Kernel drivers subsystem

Expected functions from the Drivers by the kernel includes
- ***AuDriverMain*** - The Main entrypoint for the driver
- ***AuDriverUnload*** - Clears all the resources allocated by the driver and unload it from the Kernel

Example for writing a Xeneva Kernel mode driver:

```
AU_EXTERN AU_EXPORT int AuDriverUnload() {
    /*
     * Free up all allocated resources 
     * and return
     */
}
AU_EXTERN AU_EXPORT int AuDriverMain() {
   int bus,dev, func = 0;
   uint64_t device = AuPCIEScanClass(classCode, subClassCode, &bus, &dev, &func);
   if (device == UINT32_MAX)
       return 0;
    ...Now get all PCIe information like base address, setup command register for dma, interrupt etc...
    
    /* Allocate an MSI/MSI-X signal for this device and 
       print out the result to Kernel screen */
    if (AuPCIEAllocMSI(device,36,bus,dev,func))
        SeTextOut("MSI Allocated for device %x \r\n",device);
    setvect(36, DeviceInterruptHandler); /* Here 36 vector number is used randomely, you can use your own vector number */

    AuDevice *audev = (AuDevice*)kmalloc(sizeof(AuDevice));
	audev->classCode = classCode;
	audev->subClassCode = subClassCode;
	audev->progIf = progIf;
	audev->initialized = true;
	audev->aurora_dev_class = DEVICE_CLASS_x;
	audev->aurora_driver_class = DRIVER_CLASS_x;
	AuRegisterDevice(audev);
    return 1;
} 
```
Upto here the Kernel will smoothly initialize the driver and powers up the destination hardware and registers and interrupt handler for it, to handle interrupt whenever necessary.

## devfs (The device file system)
In Xeneva, The device can be controlled through file system callbacks, as for all devices the driver creates a file and store it in the device file system, similar to Unix environment. For example to control each storage discs of AHCI controller, you can open the desired disc file from ```dev/disk0/ahci1``` to read the disc number one. Another example for reading the first SSD from NVMe controller, you can open the file from ```dev/dsik1/nvme0```. 

## Interrupt handling
During external driver initialization, the kernel temporarily disables interrupts to ensure that drivers initialize in a stable and predictable manner. However, certain drivers depend on interrupts to complete their initialization sequence. In such cases, a driver may temporarily enable interrupts during its initialization process and disable them again once initialization is complete.<br><br>
Before continuing with the initialization of the next driver, the kernel ensures that interrupts are properly disabled. This prevents unexpected interrupt handling during subsequent driver initialization stages and helps maintain a smooth and controlled boot process.<br><br>

On x86_64 systems, interrupts are disabled using `x64_cli()` and enabled using `x64_sti()`, both declared in `x86_64_lowlevel.h`.  <br><br>
On ARM64 systems, interrupts are disabled using `mark_irqs()` and enabled using `enable_irqs()`, declared in `aa64lowlevel.h`.

## Registering Interrupt handler
 The kernel maintains a list of interrupt handler functions that are invoked whenever a corresponding interrupt is triggered. Both x86_64 and ARM64 architectures follow this interrupt handling model. <br><br>
 Before an interrupt handler can receive interrupts, the corresponding interrupt number must be enabled in the interrupt controller. By default, all interrupts remain disabled until explicitly enabled. <br><br>
 ### x86_64 systems: 
 On x86_64 systems, interrupts handlers can be registered using: ```setvect(size_t vector, void(*function)(size_t vector, void* param))```
 - ``vector`` specifies the interrupt vector number.
 - ``function`` is the interrupt handler function associated with that interrupt.

 Before calling ``setvect``, the interrupt must already be enabled at both the bus level and the interrupt controller level.

 ### arm64 systems:
 On ARM64 systems, interrupt handlers can be registered using:
 ```GICRegisterSPIHandler(void* fptr, int spi)```
 - ``spi`` specifies the Shared Peripheral Interrupt (SPI) number.
 - ``fptr`` points to the interrupt handler function.

 Before calling ``GICRegisterSPIHnalder()``, the interrupt must be enabled at both the bus level and the interrupt controller level.
 At the interrupt controller level the interrupt can be enabled using : ``GICEnableSPIIRQ(uint32_t irq)`` where ``irq`` is the interrupt number.