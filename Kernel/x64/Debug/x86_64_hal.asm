; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?x86_64_hal_initialise@@YAXPEAU_KERNEL_BOOT_INFO_@@@Z ; x86_64_hal_initialise
EXTRN	x86_64_hal_init_gdt:PROC
EXTRN	?x86_64_idt_init@@YAXXZ:PROC			; x86_64_idt_init
EXTRN	?x86_64_enable_syscall_ext@@YAXXZ:PROC		; x86_64_enable_syscall_ext
EXTRN	?x86_64_init_user@@YAX_K@Z:PROC			; x86_64_init_user
EXTRN	?x86_64_get_tss@@YAPEAU_tss@@XZ:PROC		; x86_64_get_tss
EXTRN	?x86_64_hal_cpu_feature_enable@@YAXXZ:PROC	; x86_64_hal_cpu_feature_enable
EXTRN	?x86_64_initialise_syscall@@YAXXZ:PROC		; x86_64_initialise_syscall
EXTRN	x64_cli:PROC
EXTRN	x64_sti:PROC
EXTRN	?x86_64_exception_init@@YAXXZ:PROC		; x86_64_exception_init
EXTRN	?AuAPICInitialise@@YAX_N@Z:PROC			; AuAPICInitialise
EXTRN	?AuCreatePerCPU@@YAXPEAX@Z:PROC			; AuCreatePerCPU
EXTRN	?AuPerCPUSetCpuID@@YAXE@Z:PROC			; AuPerCPUSetCpuID
EXTRN	?AuPerCPUSetKernelTSS@@YAXPEAU_tss@@@Z:PROC	; AuPerCPUSetKernelTSS
EXTRN	kmalloc:PROC
EXTRN	?AuACPIInitialise@@YAXPEAX@Z:PROC		; AuACPIInitialise
pdata	SEGMENT
$pdata$?x86_64_hal_initialise@@YAXPEAU_KERNEL_BOOT_INFO_@@@Z DD imagerel $LN3
	DD	imagerel $LN3+164
	DD	imagerel $unwind$?x86_64_hal_initialise@@YAXPEAU_KERNEL_BOOT_INFO_@@@Z
pdata	ENDS
xdata	SEGMENT
$unwind$?x86_64_hal_initialise@@YAXPEAU_KERNEL_BOOT_INFO_@@@Z DD 010901H
	DD	06209H
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_hal.cpp
_TEXT	SEGMENT
cpu$ = 32
info$ = 64
?x86_64_hal_initialise@@YAXPEAU_KERNEL_BOOT_INFO_@@@Z PROC ; x86_64_hal_initialise

; 45   : void x86_64_hal_initialise(KERNEL_BOOT_INFO *info) {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 46   : 	x64_cli();

	call	x64_cli

; 47   : 
; 48   : 	x86_64_hal_init_gdt();

	call	x86_64_hal_init_gdt

; 49   : 	x86_64_idt_init();

	call	?x86_64_idt_init@@YAXXZ			; x86_64_idt_init

; 50   : 	
; 51   : 	x86_64_exception_init();

	call	?x86_64_exception_init@@YAXXZ		; x86_64_exception_init

; 52   : 
; 53   : 	AuAPICInitialise(true);

	mov	cl, 1
	call	?AuAPICInitialise@@YAX_N@Z		; AuAPICInitialise

; 54   : 
; 55   : 	x86_64_enable_syscall_ext();

	call	?x86_64_enable_syscall_ext@@YAXXZ	; x86_64_enable_syscall_ext

; 56   : 	x86_64_init_user(64);

	mov	ecx, 64					; 00000040H
	call	?x86_64_init_user@@YAX_K@Z		; x86_64_init_user

; 57   : 	x86_64_hal_cpu_feature_enable();

	call	?x86_64_hal_cpu_feature_enable@@YAXXZ	; x86_64_hal_cpu_feature_enable

; 58   : 	
; 59   : 	
; 60   :    
; 61   : 	AuACPIInitialise(info->acpi_table_pointer);

	mov	rax, QWORD PTR info$[rsp]
	mov	rcx, QWORD PTR [rax+82]
	call	?AuACPIInitialise@@YAXPEAX@Z		; AuACPIInitialise

; 62   : 
; 63   : 	x86_64_initialise_syscall();

	call	?x86_64_initialise_syscall@@YAXXZ	; x86_64_initialise_syscall

; 64   : 
; 65   : 	CPUStruc *cpu = (CPUStruc*)kmalloc(sizeof(CPUStruc));

	mov	ecx, 17
	call	kmalloc
	mov	QWORD PTR cpu$[rsp], rax

; 66   : 	cpu->cpu_id = 0;

	mov	rax, QWORD PTR cpu$[rsp]
	mov	BYTE PTR [rax], 0

; 67   : 	cpu->au_current_thread = 0;

	mov	rax, QWORD PTR cpu$[rsp]
	mov	QWORD PTR [rax+1], 0

; 68   : 	cpu->kernel_tss = NULL;

	mov	rax, QWORD PTR cpu$[rsp]
	mov	QWORD PTR [rax+9], 0

; 69   : 	AuCreatePerCPU(cpu);

	mov	rcx, QWORD PTR cpu$[rsp]
	call	?AuCreatePerCPU@@YAXPEAX@Z		; AuCreatePerCPU

; 70   : 	AuPerCPUSetCpuID(0);

	xor	ecx, ecx
	call	?AuPerCPUSetCpuID@@YAXE@Z		; AuPerCPUSetCpuID

; 71   : 	AuPerCPUSetKernelTSS(x86_64_get_tss());

	call	?x86_64_get_tss@@YAPEAU_tss@@XZ		; x86_64_get_tss
	mov	rcx, rax
	call	?AuPerCPUSetKernelTSS@@YAXPEAU_tss@@@Z	; AuPerCPUSetKernelTSS

; 72   : 	/* acpica needs problem fixing */
; 73   : 	//AuInitialiseACPISubsys(info);
; 74   : 
; 75   : 	x64_sti();

	call	x64_sti

; 76   : }

	add	rsp, 56					; 00000038H
	ret	0
?x86_64_hal_initialise@@YAXPEAU_KERNEL_BOOT_INFO_@@@Z ENDP ; x86_64_hal_initialise
_TEXT	ENDS
END
