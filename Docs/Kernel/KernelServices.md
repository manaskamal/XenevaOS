# Kernel Service calls

The XenevaOS Service Call Interface (SCI) provides a controlled gateway for user-space applications to request services from the kernel. Service calls act as the bridge between user mode and kernel mode, we can also call it as Kernel IPC calls. Service calls allows application to perform privileged operations such as file access, memory management, process control, and network communication. In XenevaOS, all user applications interact with the kernel through a well-defined set of system calls that follow a consistent calling convention and interface layout.

## Calling Convention

System calls in XenevaOS are invoked using the ``syscall`` instruction (on x86_64) or ``svc #0`` (on ARM64). <br><br>
The system call number is placed in the ``R12`` register, and arguments are passed through registers ``R13,R14,R15,RDI`` and the stack. Upon completion, the return value is stored in ``RAX`` register.

For ARM64, the system call number is placed in ``X16`` register, and arguments are passed through registers ``X0, X1, X2, X3, X4,X5`` registers. Upon completion, the return value is stored in ``X6`` register. <br><br>

Example (x86_64):
```
ExitProcess:
      mov r12, 5
	  mov r13, 0
	  mov r14, 0
	  mov r15, 0
	  mov rdi, 0
	  syscall
	  ret
```

Example (ARM64):
```
ProcessSleep:
      mov x16, 23
	  svc #0
	  mov x0, x6
	  ret
```

## Kernel Service calls

The Kernel maintains a service call dispatch table, which maps each Kernel handler function to each respective system call number.

| Service Number | Symbol Name | Description | 
|----------------|-------------|-------------|
| 0 | ``null_call`` | _Does nothing, returns 0_ |
| 1 | ``SeTextOut/UARTDebugOut`` | _Uses Kernel serial console output to print user space messages_ |
| 2 | ``PauseThread`` | _Put currently running thread to block queue_ |
|3 | ``GetThreadID`` | _Returns currently running thread ID_ |
| 4 | ``GetProcessID`` | _Returns current running process ID_ |
|5 | ``ProcessExit`` | _Terminate and deallocate all resources of current running process_ |
| 6 | ``ProcessWaitForTermination`` | _Put the currently running process into wait queue untill desired process terminates. Desired process ID is taken as argument by current process_ |
| 7 | ``CreateProcess`` | _Create a new process slot in Kernel_ |
|8 | ``ProcessLoadExec`` | _Loads an executable to provided process slot_ |
| 9| ``CreateSharedMem`` | _Create a shared memory segment into current process slot_ |
| 10 | ``ObtainSharedMem`` | _Maps and return a pointer to shared memory address which mapped to the physical addresses of provided shared memory segment_ |
| 11 | ``UnmapSharedMem`` | _Unmap a shared memory from current process, Shared memory segment is not freed unless it is detached from all attached processes_ |
|12 | ``OpenFile`` | _Open a file on Kernel and return its file descriptor allocated on the current process slot_ |





