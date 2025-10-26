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


