# XenevaOS Driver - Development

Driver softwares are important part of Operating System as it allows the OS to communicate with a hardware device or a software component. Drivers act as a translator between the OS and the device or component, enabling them to exchange data and instructions. Drivers runs in high priviledge level or the Kernel Land. The Kernel land is highly priviledged level, means it has all the power over the hardware. In XenevaOS, the driver software uses Kernel provided functions to communicate with hardware and prepares an environment for the Kernel to control the hardware. Both are inter-dependent.

## Project Configuration
Development of XenevaOS is done purely under Windows environment. Xeneva project uses Visual Studio and its compiler collection. ***Visual Studio 2013, 2019 or later*** version are recommended. **[NOTE]:Before beginning driver development, please make sure to setup Xeneva development environment, visit [Build Instructions.](../BuildInstructions.md)**<br>
- Get the XenevaOS repository
- From the repository, open the Templates folder and copy ***"MyXenevaDriver.zip*** to ***C:/Users/YourUsername/Documents/Visual Studio YourVersion/Templates/ProjectTemplates***, Replace _YourUsername_ with actual user name of your PC and _YourVersion_ with your Visual Studio version (e.g. 2013, 2019, 2022 ...etc). For example, my Visual Studio version is 2019 and my user name is 'XenevaOS' so the path would look like - ***C:/Users/XenevaOS/Documents/Visual Studio 2019/Templates/ProjectTemplates***.
- Start Visual Studio, and open ***Aurora.sln*** from the repository, or alternatively you can directly open ***Aurora.sln*** file from repository.
- In the solution explorer (_right side of visual studio window_), right click on _"Solution Aurora" -> Add -> New Project_.
- In the New Project Window, _"MyXenevaDriver"_ should appear with Xeneva logo, click on _"MyXenevaDriver"_ and click Next
- Now give a suitable name to your driver project and change the location to _"Drivers"_ folder inside _"Aurora"_ folder from XenevaOS repository, for e.g. _"../Aurora/Drivers"_ then click Create.
- Now a new project should appear in the solution explorer within the _"Solution 'Aurora' "_.
- Now right click on the newly created driver project, from the solution explorer and open properties, ***NewlyCreatedProject -> Properties***
- Now inside Properties window, Goto ***Configuration Properties -> General*** and change the ***Target Name*** to your ***desired name*** _(see [Driver Naming Convention](#driver-naming-convention) for more information)_, click Apply and Ok.
- Finally you can start coding your Xeneva driver.

## Driver Naming Convention
XenevaOS project includes certain Naming conventions.In XenevaOS driver development naming conventions plays a crucial role in supporting Backward compatibility. Early XenevaOS supported FAT32 file system with no long name entry support. XenevaOS drivers names are limited to maximum 8 characters extra three characters are reserved for extension, as FAT File system without long file name entry supports maximum 11 characters in file name. _We recommend limiting the driver name up to 8 characters_.

## Dependencies
- XenevaOS base headers 
- Kernel.lib (which is automatically created after building the Kernel 'Aurora')

## Xeneva Driver Types
Current version of XenevaOS supports only Kernel mode drivers. The Kernel mode drivers are divided into two types:
- Boot Time Drivers 
- Runtime Drivers
- USB Class Drivers

You can read detailed information about each driver in [About XenevaOS Driver](../Kernel/Drivers.md)

## Your first Xeneva Driver
Information on Initialization and Unloading of XenevaOS drivers can be found [here](../Kernel/Drivers.md#initialization). Xeneva Driver must implement two basic driver functions:
- ***AuDriverMain*** - The Main entrypoint for the driver
- ***AuDriverUnload*** - Clears all the resources allocated by the driver and unload it from the Kernel

See here for a [simple driver example.](../Kernel/Drivers.md#initialization)

## Memory Management
Memory Management is very important part of XenevaOS driver development. It's the most critical aspect of the system stability, as improper handling of memory can lead to severe crashes or unpredicatable behavior. Since drivers operate at a low level with direct access to system resources, any mistake in ___Allocation, Deallocation, or Access___ can corrupt kernel memory, causing system instability or even a complete crash. Proper use of Physical Memory Allocation/Deallocation and kernel heap Allocation/Deallocation are recommended. Memory management are divided into three parts-
- _MMIO memory mapping_ that maps hardware's physical address to Kernel virtual memory
- _Kernel Heap memory management_ helps allocating and deallocating small objects
- _Physical Memory management_ helps allocating direct physical memory allocation, as some hardware doesn't understand virtual memory. Here, P2V and V2P function plays important role, as kernel is mapped to higher half memory and lower memory mappings are cleared before entering user space. Entire physical memory is linearly mapped from 0xFFFF800000000000.

### MMIO Mappings
Once the system enters userspace the physical addresses are not accessible, trying to access physical address will cause the system to enter _Page Fault Exception_. For example consider, '0xFE000000' a physical address and is an address of some hardware. Writing to this address will cause some commands to the hardware. Say, we write the value _"1"_ to offset 0x4 of the physical address _'0xFE000000'_ which looks like ``` *(0xFE000000 + 0x04) = 1```,which will cause the hardware to enter into reset state. And suppose this entire situation happened after Xeneva entered user space and starting of user services. This will cause the system to enter _Page Fault Exception_, because before the system transition into user world, the entire physical memory mappings are cleared from the Kernel address space and the physical memory mappings are always present in lower half of address space, the lower half is completely cleared for user space memory mappings. Somewhere we need a trick to map this hardware address so that we can access the hardware through this mapped address. And here we use Xeneva MMIO Mappings.

### _Why do we clear the Lower Half_
There are two reason - 
- The kernel code and its data's are always present in higher half of the kernel's address space, and the lower half is filled with physical memory mappings. Whenever new process are created in Xeneva, they get completely new address space, and when the process occupies the CPU i.e it get executed, the address space is also switched to process's address space, and if the process's address space's higher half is not filled with Kernel code and Kernel data, immediate system failure will happen. To prevent this, whenever a new address space is created for any process, the kernel address space is directly copied to it, so that the new address space doesn't lack the kernel code and data into it.
- For security purpose, User space program will use memory from lower memory. When doing this, if it writes to any hardware area, it will cause the system to enter _Protection Fault_ or _Abort Exception_.

MMIO are mapped through '___AuMapMMIO'___  function call, ```AuMapMMIO(uint64_t physicalAddr, size_t sizeOfTheAddress)```, Once mapped successfully, it will return the virtual address where you can write to give command to or control the hardware.



## ***TODO***
_Section describing PCIe management, interrupt handling and synchronization, Memory Management, and Kernel resource allocation will be available soon_



