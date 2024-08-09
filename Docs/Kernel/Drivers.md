# Drivers in Xeneva

Drivers are specialized software components that enable an Operating System to communicate with hardware devices. In Xeneva, kernel mode drivers helps the entire OS to communicate with hardware devices. There are two types of drivers : 
- __Kernel mode drivers__
- __User mode drivers__

Current versions of Xeneva supports only _Kernel mode drivers_.

## Boot Time Drivers
Boot time drivers are the drivers that gets loaded prior to the Kernel for basic runtime of Kernel. Boot time drivers are loaded by XNLDR which loads all necessary boot drivers and maps it to _Kernel Space_ (_Higher half memory_). Some example of boot time drivers are all _Storage Drivers_ , _File System drivers_ and ACPI driver. Storage Drivers includes (_NVMe driver, AHCI/SATA Controller driver, IDE drivers and USB Mass Storage driver_). 