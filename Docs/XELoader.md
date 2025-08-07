# XELoader ( _Xeneva dynamic linker and loader_ )

XELoader is crucial component in XenevaOS, playing vital role in the process of running programs. XELoader is responsible for loading an executable file into process's memory, preparing it for execution by loading all the necessary resources for it. XELoader supports dynamic linking by loading only the required libraries and linking it to the executable during runtime. This involves finding the required libraries, mapping them into memory and resolving addresses of executable and the library. Some commonly used libraries are _XEClib - 'The Xeneva C Library', Chitralekha - ' The Xeneva Graphics, Widget and Audio library_.

## What is _Dynamic Linker and Loader_?
The dynamic linker and loader are components of modern operating systems that handle the process of preparing and executing dynamically linked executables - executables that depend on shared libraries. It does -
- Resolves external symbols used by the program at runtime.
- Links the required shared libraries with the executable after it's loaded into memory.
- Loads the program and shared libraries into memory before linking.
- Maps each segment(code, data) of the binary into the process's address space.


## Initialization
Initialization of dynamically linked program consist two steps. Whenever a new process slot is created and an executable is loaded into the process slot, the Kernel verifies if the executable is statically linked or dynamically linked. Statically linked programs are directly executed by Kernel bypassing the XELoader and dynamically linked programs are passed to XELoader. XELoader is statically linked program. If Kernel finds dynamically linked program during loading the program, immediately it dismisses the loading process and start loading XELoader. Kernel passes the name of dynamically linked program as an argument followed by other arguments for the the program to the XELoader. Once XELoader gains control, it start loading the program in User mode, during runtime.

## Loading and Linking

Everything inside XELoader is an object. Objects are of two type : _Main Executable and Libraries_. Objects holds the information about the binary and dependencies. XELoader first loads the main executable and gets the name of required library. It loads all the required library as an object and store it in the object list. During library loading stage two steps are performed :
- __Symbol resolution__ is performed by binding symbol reference of one library from another library. Practical example involves the _Chitralekha library_ has many symbol reference to _XEClib_ which needs symbol binding to be performed efficiently. 
- __Relocation__ : After loading, XELoader updates addresses of each library to the loaded memory address, so that symbol references point to the correct memory location. By default every executable has base address of 0x600000 in XenevaOS.<br><br>

XELoader at last links all the loaded libraries to the main executable by performing similar steps performed during library loading stage. The main object holds the main executable. At this stage the main _entry point_ is extracted from the main executable and stored in main object list.

> Note: Whenever you load an executable it gets linked with required dynamic libraries. Whenever another or same executable needs the same library, it doesn't get loaded from the disk rather the it get pointed to the previously loaded dynamic library until a 'data write' to that library happens. If a 'data write' happens the exact copy of that library is created, this is called "Copy-On-Write" algorithm.

## Control Transfer
Transferring the control to the main executable is only performed after loading and linking of the main program and libraries are done efficiently. Failure of loading and linking results in program crash. Before transferring control to the program, XELoader updates the signal table of the current process with Default Signal Handler. Finally XELoader transfers the control to the program by calling the _Main Entry Point_ from the main object list.

## Technical Overview
XELoader uses PE image format for executable and libraries, i.e it is only limited to loading of PE image format. _Elf_ format can also be added for supporting Elf binaries. XELoader itself is a statically linked binary means it doesn't depend on shared library, everything is build statically inside XELoader. 

There are two-level of loader built into Xeneva :
- The Kernel level loader (_Kernel space static executable loader_)
- The XELoader (_Userspace dynamic linker and loader_)

Whenever Xeneva encounters system call _```_KeProcessLoadExec```_ with the executable file path and name, it immediately loads the executable from the path and verifies if it's PE formatted executable, not a library. Kernel level loader is only limited to load statically linked executable files not dynamically linked executable nor library. If it's a executable file, then it further check if it's dynamically linked or statically. Static executable files can be directly executed from Kernel level loader, which means it'll map each section of the executable to process's address space and transfer the control to the executable entry point. For dynamically linked executable file, an extra layer is placed in between. Kernel level loader load XELoader (_which is a statically linked loader_) and put the desired _filename and its path_ into XELoader argument list and transfer the control to XELoader. XELoader then get executed into user space and start the process of loading the executable file that kernel level loader provided. XELoader then further resolve external symbols by loading and linking required shared libraries.
For example here are the steps how it works:

| Steps  | Description|
|--------|------------|
| (a)  _```_KeProcessLoadExec(proc, "/calc.exe",0,NULL);```_| The _```_KeProcessLoadExec```_ system calls carries a pointer to newly created process slot and the executable file name that needs to be loaded into newly created process slot.
| (b) _```AuLoadExecToProcess(proc,"/calc.exe",0,NULL);```_ | Get called in Kernel space and it receives the parameters from _```_KeProcessLoadExec```_.
|(c) _```strcmp(filename,"exe") == 0```_ | Check the extension of the given filename if it's ended with '.exe', because Kernel level loader is limited to loading static executables only.
|(d) _```AuVFSNodeReadBlock(fsys,filename,(uint64_t*)V2P((uint64_t)buffer))```_ | If the filename ends with '.exe' then load the file into current process address space.
|(e)_```if(AuPEFileIsDynamicallyLinked(buffer));```_ | Check if the executable is dynamically linked or statically, if dynamically linked, the loading of this executable is discarded and XELoader is loaded.
|(f) _```AuCreateKthread(AuProcessEntUser,...)```_ | Create a kernel thread with entry point to _```AuProcessEntUser```_ which is a helper function that help jump to mapped user thread, i.e XELoader in this case, if executable was dynamically linked.

After above step is executed successfully, XELoader will have the name of executable (_```"/calc.exe"```, in this case_ ) in its argument list. XELoader can load the executable, resolve external symbols, link required libraries during runtime,..etc and start executing the executable file.

