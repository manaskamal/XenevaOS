# Memory Management in Xeneva

Memory Management is the most important part of XenevaOS, as it is a process of controlling and coordinating installed available memory, assigining portions called blocks to various running programs and deallocate it when they are done. Xeneva manages memory through two subsystem, _Physical Memory and Virtual Memory_, Both are very important part of Xeneva's Memory Management subsystem and many memory management layers heavily depends on Physical Memory Manager and Virtual Memory Manager. Three components of Memory Managers are: 
- Physical Memory Manager
- Virtual Memory Manager
- Shared Memory Manager

## Physical Memory Manager
Xeneva uses _'Paging'_ mechanism to manage available installed memory, as _'Paging'_ is a efficient memory management scheme that eliminates the need for contiguous allocation of physical memory, thus solving the problem of fitting varying-sized memory chunks onto the backing store. By default Xeneva uses 4KiB (_4096 bytes_) pages. The initialization process of Physical Memory Manager involves two phases: _The XNLDR phase and The Kernel phase_.<br><br>
 The XNLDR phase prepares a basic Physical Memory Manager in order to load the kernel and its runtime resources and the Kernel phase prepares a full time Physical Memory Manager. The data structure used to manage Physical Memory in XNLDR is _dereferenced pointer_ and the data structure used in Kernel is Bitmap allocation. 

- __XNLDR Phase__: XNLDR simply go through _EFI_MEMORY_DESCRIPTOR to skip all the non usable memory areas and uses _EfiConventionalMemory_ to store the usable memory in an _incremental dereferenced pointer_. Allocation of Physical Memory in XNLDR involves decrementing the dereferenced pointer that is being used to store all _EfiConventionalMemory_ from the last and storing the allocated physical block number inside an another dereferenced pointer called '_allocated_stack_'. The XNLDR physical memory manager is used untill the kernel and its runtime resources are fully loaded into memory. When XNLDR passes control to 'Kernel'. '_allocated_stack_' pointer is passed to the Kernel in order to ensure that Kernel doesn't overwrite any blocks that is allocated by XNLDR.
- __Kernel Phase__: Kernel uses Bitmap allocation/deallocation scheme to manage Physical Memory. It initialises available and usable memory by scanning the Memory map provided by XNLDR and locks all memories that are unusable. The Kernel also locks all memories that are used by XNLDR which is passed through '_allocated_stack_' pointer.

## Virtual Memory Manager
The Virtual Memory manager in XenevaOS involves two phases same like Physical Memory Manager, _The XNLDR phase & the Kernel Phase_. XNLDR creates a temporary paging structure and loads the Kernel into higher half address. The Kernel creates a new paging structure with additional steps like __Linear Mapping the entire Physical Memory and Clearing the Lower Half section__. <br><br>
__Linear Mapping the entrire Physical Memory__ involves the mapping of Physical address linearly from _0xFFFF800000000000_. For example if physical allocation at address _0x11B5000_ is done than it would look like _0xFFFF8000011B5000_. This is done via special functions _P2V and V2P_. The _P2V_ function converts a physical address to its linear virtual address and _V2P_ converts a linear virtual address to its actual physical address. Linear mapping is important for process management. The lower half of Virtual Memory is reserved for process and the Higher Half is fixed for Kernel and its resources and is shared among all processes. <br><br>
For a process an entire new Virtual Memory is created. Where Kernel code is shared between all processes through the higher half section. Benifits of creating an entire new _Address space_ for each process is it isolates the process from accessing a memory area from another process. Linear mapping helps in isolating the address spaces of different processes.

__Clearing the lower half section__: Clearing the lower half section is only possible if the Kernel is using Linear addresses for allocating Physical memory for any reason. For example if a driver would allocate a physical memory through _AuPmmngrAlloc_ function, the function must uses Linear address to allocate a physical block. When Virtual Memory Manager initializes itself it automatically store the information whether the memory manager has linearly mapped all the physical memory or not. This helps the Physical Memory Manager to decide whether to allocate Physical Block from linear address or non linear address. If anywhere inside the kernel, a physical block is allocated from non-linear space, clearing the lower half section would cause immediate crash.<br>
The __AuVmmngrBootFree__ function clear out the lower half section and store the new paging structure into CR3 register.

__MMIO Memory Area__: The Virtual Address starting from _0xFFFFFF1000000000_. This region is reserved for Hardware to map its device registers into the virtual address space. This region is also marked as non-cacheable region. The __'AuMapMMIO'__ function is used to map device memories into _MMIO Region_. 

__The External Functions of Virtual Memory Manager__: External functions in Virtual Memory Manager are marked with _'Ex'_ suffix with their function name. There are only two external function currently available, __AuMapPageEx__ and __AuGetPhysicalAddressEx__.
- __AuMapPageEx__: In Xeneva, mapping a physical address to a virtual address from one address space to another address space is possible through this function. The destination address space need not to be switched from current address space. Switching address space involves writing the address space to _control register_. Xeneva doesn't follow fork-exec model of Unix, whenever a process is created from current process, an entire new new process slot is created rather than copying the current process and replacing it with new executable. Whenever new process is created from running process via service call, __AuMapPageEx__ is used for mapping physical memory to virtual memory in newly created process's address space without switching to that address space even temporarily.

- __AuGetPhysicalAddressEx__: Similar to __AuMapPageEx__ getting a physical address of a virtual address from one address space to another is also possible, through this function. This function is used in _AuProcessClean_ which is used by _init_ process, _init_ process is responsible for cleaning all dead processes.

## Shared Memory Manager
Between multiple process a shared memory plays a very important role in passing data with low latency. ProcessA may create a shared memory segment with a specific key, and pass that key to ProcessB through arguments or inter-process communication mechanism. ProcessB would create a new shared memory segment with the provided key and access the shared memory segment. Only the common thing that both the processes have is the physical address. ProcessA create a completely fresh shared memory segment with a physical block backend to it, ProcessB create a shared memory segment with the key provided by ProcessA. The difference is in ProcessB creating the shared memory segment, the Shared Memory Manager looks for the key that ProcessB used for creating a new segment, if it is already present in Shared Memory Manager, the Shared Memory Manager simply maps the physical addresses from the already created segment to the newly created segment. Through this ProcessA can pass information to ProcessB with low latency. <br><br>
Shared Memory is very useful for low latency information passing between two or more then two processes. The Display Server in XenevaOS and all the Graphical Application heavily depends on the Shared Memory Manager. Shared Memory Manager is also used between Audio Server and its client processes.<br>

Shared memory segment is created with the function __AuCreateSHM__ and shareable memory is obtained a from the newly created segment through the __AuSHMObtainMem__ function.

__AuCreateSHM__: Creates a new shared memory segment's parameters:
- __AuProcess* proc__ -- the current process that wants to create a new shared memory segment
- __uint16_t key__ -- the unique key that will be used by this process or another process to identify this memory segment
- __size_t sz__ -- size of the memory segment.
- __uint8_t flags__ -- any protection flags that will be considered by the shared memory manager while creating shared memory segment for another process with this key

__AuSHMObtainMem__: Obtains a memory from newly created shared memory segment. Its parameters are:
- __AuProcess* proc__ - the same process that created the shared memory segment and wants to obtain a memory from that segment.
- __uint16_t id__ -- Memory Segment id returned by __AuCreateSHM__ function. 
- __void* shmaddr__ -- This should be NULL if the shared memory segment should not be mapped to a fixed virtual address.
- __int shmflg__ -- any protection flags or controlling flags that should be considered by shared memory manager while obtaining memory from the provided shared memory segment. 
