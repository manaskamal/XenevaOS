# XELoader ( _Xeneva dynamic linker and loader_ )

XELoader is crucial component in XenevaOS, playing vital role in the process of running programs. XELoader is responsible for loading an executable file into process's memory, preparing it for execution by loading all the necessary resources for it. XELoader supports dynamic linking by loading only the required libraries and linking it to the executable during runtime. This involves finding the required libraries, mapping them into memory and resolving addresses of executable and the library. Some commonly used libraries are _XEClib - 'The Xeneva C Library', Chitralekha - ' The Xeneva Graphics, Widget and Audio library_.

## Initialization
Initialization of dynamically linked program consist two steps. Whenever a new process slot is created and an executable is loaded into the process slot, the Kernel verifies if the executable is statically linked or dynamically linked. Statically linked programs are directly executed by Kernel bypassing the XELoader and dynamically linked programs are passed to XELoader. XELoader is statically linked program. If Kernel finds dynamically linked program during loading the program, immediately it dismisses the loading process and start loading XELoader. Kernel passes the name of dynamically linked program as an argument followed by other arguments for the the program to the XELoader. Once XELoader gains control, it start loading the program in User mode, during runtime.

## Loading and Linking

Everything inside XELoader is an object. Objects are of two type : _Main Executable and Libraries_. Objects holds the information about the binary and dependencies. XELoader first loads the main executable and gets the name of required library. It loads all the required library as an object and store it in the object list. During library loading stage two steps are performed :
- __Symbol resolution__ is performed by binding symbol reference of one library from another library. Practical example involves the _Chitralekha library_ has many symbol reference to _XEClib_ which needs symbol binding to be performed efficiently. 
- __Relocation__ : After loading, XELoader updates addresses of each library to the loaded memory address, so that symbol references point to the correct memory location. By default every executable has base address of 0x600000 in XenevaOS.<br><br>

XELoader at last links all the loaded libraries to the main executable by performing similar steps performed during library loading stage. The main object holds the main executable. At this stage the main _entry point_ is extracted from the main executable and stored in main object list.

## Control Transfer
Transferring the control to the main executable is only performed after loading and linking of the main program and libraries are done efficiently. Failure of loading and linking results in program crash. Before transferring control to the program, XELoader updates the signal table of the current process with Default Signal Handler. Finally XELoader transfers the control to the program by calling the _Main Entry Point_ from the main object list.
