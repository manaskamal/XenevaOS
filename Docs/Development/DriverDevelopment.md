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
_will be available soon_



## ***TODO***
_Section describing PCIe management, interrupt handling and synchronization, Memory Management, and Kernel resource allocation will be available soon_



