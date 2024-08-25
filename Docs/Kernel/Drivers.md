# Drivers in Xeneva

Drivers are specialized software components that enable an Operating System to communicate with hardware devices. In Xeneva, kernel mode drivers helps the entire OS to communicate with hardware devices. There are two types of drivers : 
- __Kernel mode drivers__
- __User mode drivers__

Current versions of Xeneva supports only _Kernel mode drivers_.

## Boot Time Drivers
Boot time drivers are the drivers that gets loaded prior to the Kernel for basic runtime of Kernel. Boot time drivers are loaded by XNLDR which loads all necessary boot drivers and maps it to _Kernel Space_ (_Higher half memory_). Some example of boot time drivers are all _Storage Drivers_ , _File System drivers_ and ACPI driver. Storage Drivers includes (_NVMe driver, AHCI/SATA Controller driver, IDE drivers and USB Mass Storage driver_). <br>
Xeneva Kernel depends on _Storage Drivers_ and _File System_ drivers to further run the system. The drivers are loaded and mapped to Kernel Space by XNLDR and the address is passed to the kernel through _Boot Information_. Xeneva Kernel includes special functions that handles boot time drivers seperately from runtime drivers. The function _AuBootDriverInitialise_ starts loading of all boot drivers. Linking of boot drivers to kernel functions is done internally in the kernel. 

## Runtime Drivers
Once boot drivers like _Storage Drivers_ are loaded and initialised successfully, kernel uses those drivers to further load and start runtime drivers. Runtime Drivers includes the _Network Drivers_, _Sound Drivers_, _Graphics Drivers_, ..etc. _Network Drivers includes Ethernet Drivers_. Sound Drivers includes _High Definition Audio Driver_ ..etc.