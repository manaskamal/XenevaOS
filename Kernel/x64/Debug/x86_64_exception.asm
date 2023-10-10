; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

CONST	SEGMENT
$SG4100	DB	0aH, 'Alignment Check Fault at address ', 00H
	ORG $+5
$SG4108	DB	0aH, 'Machine Check Abort', 00H
	ORG $+3
$SG4118	DB	0aH, ' SIMD FPU Faul ', 0dH, 0aH, 't', 00H
	ORG $+4
$SG4119	DB	0aH, '__CPU Informations__ ', 0aH, 00H
$SG4120	DB	' RIP -> %x ', 0aH, 00H
	ORG $+3
$SG4121	DB	' RSP -> %x ', 0aH, 00H
	ORG $+3
$SG4122	DB	' RFLAGS -> %x ', 0aH, 00H
$SG4123	DB	' MXCSR bit  -- ', 00H
$SG3888	DB	0dH, 0aH, ' ***ARCH x86_64 : Kernel Panic!!! *** ', 0dH, 0aH
	DB	00H
	ORG $+5
$SG3889	DB	'[Aurora Kernel]: We are sorry to say that, a processor i'
	DB	'nvalid exception has occured ', 0dH, 0aH, 00H
$SG3891	DB	'[Aurora Kernel]: Below is the code of exception ', 0dH, 0aH
	DB	00H
	ORG $+5
$SG3890	DB	'[Aurora Kernel]: please inform it to the master of the k'
	DB	'ernel ', 0dH, 0aH, 00H
	ORG $+3
$SG3893	DB	' %s ', 0dH, 0aH, 00H
	ORG $+5
$SG3892	DB	'[Aurora Kernel]: Current Processor id -> %d ', 0dH, 0aH, 00H
	ORG $+1
$SG3900	DB	0aH, 'Divide by 0', 00H
	ORG $+3
$SG3901	DB	'Divide by 0 ', 0dH, 0aH, 00H
	ORG $+1
$SG3902	DB	'__PROCESSOR_DATA__ ', 0dH, 0aH, 00H
	ORG $+2
$SG3903	DB	'RIP -> %x ', 0dH, 0aH, 00H
	ORG $+3
$SG3904	DB	'RSP -> %x ', 0dH, 0aH, 00H
	ORG $+3
$SG3905	DB	'RFLAGS -> %x ', 0dH, 0aH, 00H
$SG3915	DB	0aH, 'Single Step Trap', 00H
	ORG $+6
$SG3923	DB	0aH, 'NMI [Non-Muskable-Interrupt] Trap', 00H
	ORG $+5
$SG3931	DB	0aH, 'Breakpoint Trap', 00H
	ORG $+7
$SG3939	DB	0aH, 'Overflow Trap', 00H
	ORG $+1
$SG3947	DB	0aH, 'Bound Check Fault', 00H
	ORG $+5
$SG3957	DB	'Invalid Opcode Fault ', 0dH, 0aH, 00H
$SG3958	DB	'Invalid Opcode Fault ', 0dH, 0aH, 00H
$SG3959	DB	'__PROCESSOR TRACE__ ', 0dH, 0aH, 00H
	ORG $+1
$SG3960	DB	'RIP -> %x', 0aH, 00H
	ORG $+5
$SG3961	DB	'Stack -> %x', 0aH, 00H
	ORG $+3
$SG3962	DB	'RFLAGS -> %x', 0aH, 00H
	ORG $+2
$SG3963	DB	'CS -> %x', 0aH, 00H
	ORG $+6
$SG3964	DB	'SS -> %x', 0aH, 00H
	ORG $+6
$SG3972	DB	0aH, 'No Device Fault', 00H
	ORG $+7
$SG3980	DB	0aH, 'Double Fault Abort', 00H
	ORG $+4
$SG3988	DB	0aH, 'Invalid TSS Fault ', 00H
	ORG $+4
$SG3996	DB	0aH, 'No Segment Fault', 00H
	ORG $+6
$SG4004	DB	0aH, 'Stack Fault at ', 00H
	ORG $+7
$SG4014	DB	'Genral Protection Fault ', 0dH, 0aH, 00H
	ORG $+5
$SG4015	DB	'General Protection Fault ', 0dH, 0aH, 00H
	ORG $+4
$SG4016	DB	'__PROCESSOR TRACE__ ', 0dH, 0aH, 00H
	ORG $+1
$SG4017	DB	'RIP -> %x ', 0dH, 0aH, 00H
	ORG $+3
$SG4018	DB	'Stack -> %x ', 0dH, 0aH, 00H
	ORG $+1
$SG4019	DB	'RFLAGS -> %x ', 0dH, 0aH, 00H
$SG4020	DB	'CS -> %x, SS -> %x ', 0dH, 0aH, 00H
	ORG $+2
$SG4021	DB	'Current thread ->id %d , %s', 0dH, 0aH, 00H
	ORG $+2
$SG4043	DB	'Thr has returnable signal ', 0dH, 0aH, 00H
	ORG $+3
$SG4054	DB	'Process pid -> %d ', 0dH, 0aH, 00H
	ORG $+3
$SG4055	DB	'Process name -> %s ', 0dH, 0aH, 00H
	ORG $+2
$SG4056	DB	'Page Fault !! ', 0dH, 0aH, 00H
	ORG $+7
$SG4062	DB	'Page Not Present ', 0dH, 0aH, 00H
	ORG $+4
$SG4065	DB	'Read/Write ', 0dH, 0aH, 00H
	ORG $+2
$SG4068	DB	'VPage rw -> %d , user -> %d ', 0dH, 0aH, 00H
	ORG $+1
$SG4069	DB	'VPage phys1-> %x, phys2-> %x ', 0dH, 0aH, 00H
$SG4072	DB	'User bit not set ', 0dH, 0aH, 00H
	ORG $+4
$SG4075	DB	'Reserved page ', 0dH, 0aH, 00H
	ORG $+7
$SG4078	DB	'Invalid page ', 0dH, 0aH, 00H
$SG4079	DB	'Virtual Address -> %x ', 0dH, 0aH, 00H
	ORG $+7
$SG4080	DB	'Virtual Address aligned -> %x ', 0dH, 0aH, 00H
	ORG $+7
$SG4081	DB	'RSP -> %x ', 0dH, 0aH, 00H
	ORG $+3
$SG4082	DB	'RIP -> %x ', 0dH, 0aH, 00H
	ORG $+3
$SG4083	DB	'CS -> %x, SS -> %x ', 0dH, 0aH, 00H
	ORG $+2
$SG4092	DB	0aH, 'FPU Fault', 00H
CONST	ENDS
PUBLIC	?x86_64_exception_init@@YAXXZ			; x86_64_exception_init
PUBLIC	?panic@@YAXPEBDZZ				; panic
PUBLIC	?divide_by_zero_fault@@YAX_KPEAX@Z		; divide_by_zero_fault
PUBLIC	?single_step_trap@@YAX_KPEAX@Z			; single_step_trap
PUBLIC	?nmi_trap@@YAX_KPEAX@Z				; nmi_trap
PUBLIC	?breakpoint_trap@@YAX_KPEAX@Z			; breakpoint_trap
PUBLIC	?overflow_trap@@YAX_KPEAX@Z			; overflow_trap
PUBLIC	?bounds_check_fault@@YAX_KPEAX@Z		; bounds_check_fault
PUBLIC	?invalid_opcode_fault@@YAX_KPEAX@Z		; invalid_opcode_fault
PUBLIC	?no_device_fault@@YAX_KPEAX@Z			; no_device_fault
PUBLIC	?double_fault_abort@@YAX_KPEAX@Z		; double_fault_abort
PUBLIC	?invalid_tss_fault@@YAX_KPEAX@Z			; invalid_tss_fault
PUBLIC	?no_segment_fault@@YAX_KPEAX@Z			; no_segment_fault
PUBLIC	?stack_fault@@YAX_KPEAX@Z			; stack_fault
PUBLIC	?general_protection_fault@@YAX_KPEAX@Z		; general_protection_fault
PUBLIC	?page_fault@@YAX_KPEAX@Z			; page_fault
PUBLIC	?fpu_fault@@YAX_KPEAX@Z				; fpu_fault
PUBLIC	?alignment_check_fault@@YAX_KPEAX@Z		; alignment_check_fault
PUBLIC	?machine_check_abort@@YAX_KPEAX@Z		; machine_check_abort
PUBLIC	?simd_fpu_fault@@YAX_KPEAX@Z			; simd_fpu_fault
EXTRN	AuGetCurrentThread:PROC
EXTRN	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z:PROC ; AuProcessFindThread
EXTRN	?AuPerCPUGetCpuID@@YAEXZ:PROC			; AuPerCPUGetCpuID
EXTRN	x64_cli:PROC
EXTRN	x64_read_cr2:PROC
EXTRN	AuVmmngrGetPage:PROC
EXTRN	AuGetPhysicalAddress:PROC
EXTRN	memcpy:PROC
EXTRN	setvect:PROC
EXTRN	kfree:PROC
EXTRN	SeTextOut:PROC
pdata	SEGMENT
$pdata$?x86_64_exception_init@@YAXXZ DD imagerel $LN3
	DD	imagerel $LN3+312
	DD	imagerel $unwind$?x86_64_exception_init@@YAXXZ
$pdata$?panic@@YAXPEBDZZ DD imagerel $LN3
	DD	imagerel $LN3+116
	DD	imagerel $unwind$?panic@@YAXPEBDZZ
$pdata$?divide_by_zero_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+135
	DD	imagerel $unwind$?divide_by_zero_fault@@YAX_KPEAX@Z
$pdata$?single_step_trap@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+48
	DD	imagerel $unwind$?single_step_trap@@YAX_KPEAX@Z
$pdata$?nmi_trap@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?nmi_trap@@YAX_KPEAX@Z
$pdata$?breakpoint_trap@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?breakpoint_trap@@YAX_KPEAX@Z
$pdata$?overflow_trap@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?overflow_trap@@YAX_KPEAX@Z
$pdata$?bounds_check_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?bounds_check_fault@@YAX_KPEAX@Z
$pdata$?invalid_opcode_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+177
	DD	imagerel $unwind$?invalid_opcode_fault@@YAX_KPEAX@Z
$pdata$?no_device_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?no_device_fault@@YAX_KPEAX@Z
$pdata$?double_fault_abort@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?double_fault_abort@@YAX_KPEAX@Z
$pdata$?invalid_tss_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?invalid_tss_fault@@YAX_KPEAX@Z
$pdata$?no_segment_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?no_segment_fault@@YAX_KPEAX@Z
$pdata$?stack_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?stack_fault@@YAX_KPEAX@Z
$pdata$?general_protection_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+215
	DD	imagerel $unwind$?general_protection_fault@@YAX_KPEAX@Z
$pdata$?page_fault@@YAX_KPEAX@Z DD imagerel $LN22
	DD	imagerel $LN22+873
	DD	imagerel $unwind$?page_fault@@YAX_KPEAX@Z
$pdata$?fpu_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?fpu_fault@@YAX_KPEAX@Z
$pdata$?alignment_check_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?alignment_check_fault@@YAX_KPEAX@Z
$pdata$?machine_check_abort@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+38
	DD	imagerel $unwind$?machine_check_abort@@YAX_KPEAX@Z
$pdata$?simd_fpu_fault@@YAX_KPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+135
	DD	imagerel $unwind$?simd_fpu_fault@@YAX_KPEAX@Z
pdata	ENDS
xdata	SEGMENT
$unwind$?x86_64_exception_init@@YAXXZ DD 010401H
	DD	04204H
$unwind$?panic@@YAXPEBDZZ DD 011801H
	DD	04218H
$unwind$?divide_by_zero_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0620eH
$unwind$?single_step_trap@@YAX_KPEAX@Z DD 010e01H
	DD	0620eH
$unwind$?nmi_trap@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?breakpoint_trap@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?overflow_trap@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?bounds_check_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?invalid_opcode_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0620eH
$unwind$?no_device_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?double_fault_abort@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?invalid_tss_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?no_segment_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?stack_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?general_protection_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0620eH
$unwind$?page_fault@@YAX_KPEAX@Z DD 021101H
	DD	0150111H
$unwind$?fpu_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?alignment_check_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?machine_check_abort@@YAX_KPEAX@Z DD 010e01H
	DD	0420eH
$unwind$?simd_fpu_fault@@YAX_KPEAX@Z DD 010e01H
	DD	0620eH
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
frame$ = 32
v$ = 64
p$ = 72
?simd_fpu_fault@@YAX_KPEAX@Z PROC			; simd_fpu_fault

; 269  : void simd_fpu_fault(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 270  : 	x64_cli();

	call	x64_cli

; 271  : 	interrupt_stack_frame *frame = (interrupt_stack_frame*)p;

	mov	rax, QWORD PTR p$[rsp]
	mov	QWORD PTR frame$[rsp], rax

; 272  : 	panic("\n SIMD FPU Faul \r\nt");

	lea	rcx, OFFSET FLAT:$SG4118
	call	?panic@@YAXPEBDZZ			; panic

; 273  : 	SeTextOut("\n__CPU Informations__ \n");

	lea	rcx, OFFSET FLAT:$SG4119
	call	SeTextOut

; 274  : 	SeTextOut(" RIP -> %x \n", frame->rip);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+16]
	lea	rcx, OFFSET FLAT:$SG4120
	call	SeTextOut

; 275  : 	SeTextOut(" RSP -> %x \n", frame->rsp);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+40]
	lea	rcx, OFFSET FLAT:$SG4121
	call	SeTextOut

; 276  : 	SeTextOut(" RFLAGS -> %x \n", frame->rflags);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+32]
	lea	rcx, OFFSET FLAT:$SG4122
	call	SeTextOut

; 277  : 	SeTextOut(" MXCSR bit  -- ");

	lea	rcx, OFFSET FLAT:$SG4123
	call	SeTextOut
$LN2@simd_fpu_f:

; 278  : 	for (;;);

	jmp	SHORT $LN2@simd_fpu_f

; 279  : }

	add	rsp, 56					; 00000038H
	ret	0
?simd_fpu_fault@@YAX_KPEAX@Z ENDP			; simd_fpu_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
v$ = 48
p$ = 56
?machine_check_abort@@YAX_KPEAX@Z PROC			; machine_check_abort

; 262  : void machine_check_abort(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 263  : 	x64_cli();

	call	x64_cli

; 264  : 	panic("\nMachine Check Abort");

	lea	rcx, OFFSET FLAT:$SG4108
	call	?panic@@YAXPEBDZZ			; panic
$LN2@machine_ch:

; 265  : 	for (;;);

	jmp	SHORT $LN2@machine_ch

; 266  : }

	add	rsp, 40					; 00000028H
	ret	0
?machine_check_abort@@YAX_KPEAX@Z ENDP			; machine_check_abort
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
v$ = 48
p$ = 56
?alignment_check_fault@@YAX_KPEAX@Z PROC		; alignment_check_fault

; 255  : void alignment_check_fault(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 256  : 	x64_cli();

	call	x64_cli

; 257  : 	panic("\nAlignment Check Fault at address ");

	lea	rcx, OFFSET FLAT:$SG4100
	call	?panic@@YAXPEBDZZ			; panic
$LN2@alignment_:

; 258  : 	for (;;);

	jmp	SHORT $LN2@alignment_

; 259  : }

	add	rsp, 40					; 00000028H
	ret	0
?alignment_check_fault@@YAX_KPEAX@Z ENDP		; alignment_check_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
vector$ = 48
p$ = 56
?fpu_fault@@YAX_KPEAX@Z PROC				; fpu_fault

; 247  : void fpu_fault(size_t vector, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 248  : 	x64_cli();

	call	x64_cli

; 249  : 	panic("\nFPU Fault");

	lea	rcx, OFFSET FLAT:$SG4092
	call	?panic@@YAXPEBDZZ			; panic
$LN2@fpu_fault:

; 250  : 	for (;;);

	jmp	SHORT $LN2@fpu_fault

; 251  : }

	add	rsp, 40					; 00000028H
	ret	0
?fpu_fault@@YAX_KPEAX@Z ENDP				; fpu_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
_mapped$ = 32
frame$ = 40
thr$ = 48
sig$1 = 56
tv68 = 64
proc$ = 72
vpage$2 = 80
id$ = 88
vaddr_aligned$ = 96
present$ = 104
resv$ = 108
us$ = 112
rw$ = 116
vaddr_$ = 120
vaddr$ = 128
ctx$3 = 136
phys$4 = 144
vector$ = 176
param$ = 184
?page_fault@@YAX_KPEAX@Z PROC				; page_fault

; 173  : void page_fault(size_t vector, void* param){

$LN22:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 168				; 000000a8H

; 174  : 	x64_cli();

	call	x64_cli

; 175  : 	interrupt_stack_frame *frame = (interrupt_stack_frame*)param;

	mov	rax, QWORD PTR param$[rsp]
	mov	QWORD PTR frame$[rsp], rax

; 176  : 	void* vaddr = (void*)x64_read_cr2();

	call	x64_read_cr2
	mov	QWORD PTR vaddr$[rsp], rax

; 177  : 
; 178  : 	int present = !(frame->error & 0x1);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rax, QWORD PTR [rax+8]
	and	rax, 1
	test	rax, rax
	jne	SHORT $LN20@page_fault
	mov	DWORD PTR tv68[rsp], 1
	jmp	SHORT $LN21@page_fault
$LN20@page_fault:
	mov	DWORD PTR tv68[rsp], 0
$LN21@page_fault:
	mov	eax, DWORD PTR tv68[rsp]
	mov	DWORD PTR present$[rsp], eax

; 179  : 	int rw = frame->error & 0x2;

	mov	rax, QWORD PTR frame$[rsp]
	mov	rax, QWORD PTR [rax+8]
	and	rax, 2
	mov	DWORD PTR rw$[rsp], eax

; 180  : 	int us = frame->error & 0x4;

	mov	rax, QWORD PTR frame$[rsp]
	mov	rax, QWORD PTR [rax+8]
	and	rax, 4
	mov	DWORD PTR us$[rsp], eax

; 181  : 	int resv = frame->error & 0x8;

	mov	rax, QWORD PTR frame$[rsp]
	mov	rax, QWORD PTR [rax+8]
	and	rax, 8
	mov	DWORD PTR resv$[rsp], eax

; 182  : 	int id = frame->error & 0x10;

	mov	rax, QWORD PTR frame$[rsp]
	mov	rax, QWORD PTR [rax+8]
	and	rax, 16
	mov	DWORD PTR id$[rsp], eax

; 183  : 
; 184  : 	
; 185  : 	AuThread* thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR thr$[rsp], rax

; 186  : 	
; 187  : 	/* check for signal */
; 188  : 	if (!thr) {

	cmp	QWORD PTR thr$[rsp], 0
	jne	SHORT $LN17@page_fault

; 189  : 		goto skip;

	jmp	$LN16@page_fault
	jmp	$skip$23
$LN17@page_fault:

; 190  : 	}
; 191  : 	if (thr->returnableSignal) {

	mov	rax, QWORD PTR thr$[rsp]
	cmp	QWORD PTR [rax+627], 0
	je	$LN15@page_fault

; 192  : 		SeTextOut("Thr has returnable signal \r\n");

	lea	rcx, OFFSET FLAT:$SG4043
	call	SeTextOut

; 193  : 		Signal* sig = (Signal*)thr->returnableSignal;

	mov	rax, QWORD PTR thr$[rsp]
	mov	rax, QWORD PTR [rax+627]
	mov	QWORD PTR sig$1[rsp], rax

; 194  : 		x86_64_cpu_regs_t* ctx = (x86_64_cpu_regs_t*)(thr->frame.kern_esp - sizeof(x86_64_cpu_regs_t));

	mov	rax, QWORD PTR thr$[rsp]
	mov	rax, QWORD PTR [rax+200]
	sub	rax, 160				; 000000a0H
	mov	QWORD PTR ctx$3[rsp], rax

; 195  : 		memcpy(ctx, sig->signalStack, sizeof(x86_64_cpu_regs_t));

	mov	r8d, 160				; 000000a0H
	mov	rax, QWORD PTR sig$1[rsp]
	mov	rdx, QWORD PTR [rax+4]
	mov	rcx, QWORD PTR ctx$3[rsp]
	call	memcpy

; 196  : 		memcpy(&thr->frame, sig->signalState, sizeof(AuThreadFrame));

	mov	rax, QWORD PTR thr$[rsp]
	mov	r8d, 216				; 000000d8H
	mov	rcx, QWORD PTR sig$1[rsp]
	mov	rdx, QWORD PTR [rcx+12]
	mov	rcx, rax
	call	memcpy

; 197  : 		kfree(sig->signalState);

	mov	rax, QWORD PTR sig$1[rsp]
	mov	rcx, QWORD PTR [rax+12]
	call	kfree

; 198  : 		kfree(sig->signalStack);

	mov	rax, QWORD PTR sig$1[rsp]
	mov	rcx, QWORD PTR [rax+4]
	call	kfree

; 199  : 		kfree(sig);

	mov	rcx, QWORD PTR sig$1[rsp]
	call	kfree

; 200  : 		thr->returnableSignal = NULL;

	mov	rax, QWORD PTR thr$[rsp]
	mov	QWORD PTR [rax+627], 0

; 201  : 		return;

	jmp	$LN18@page_fault
$LN15@page_fault:

; 202  : 	}
; 203  : 
; 204  : 	AuProcess *proc = NULL;

	mov	QWORD PTR proc$[rsp], 0

; 205  : 	if (thr) {

	cmp	QWORD PTR thr$[rsp], 0
	je	SHORT $LN14@page_fault

; 206  : 		proc = AuProcessFindThread(thr);

	mov	rcx, QWORD PTR thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR proc$[rsp], rax

; 207  : 		if (proc) {

	cmp	QWORD PTR proc$[rsp], 0
	je	SHORT $LN13@page_fault

; 208  : 			SeTextOut("Process pid -> %d \r\n", proc->proc_id);

	mov	rax, QWORD PTR proc$[rsp]
	mov	edx, DWORD PTR [rax]
	lea	rcx, OFFSET FLAT:$SG4054
	call	SeTextOut

; 209  : 			SeTextOut("Process name -> %s \r\n", proc->name);

	mov	rax, QWORD PTR proc$[rsp]
	add	rax, 4
	mov	rdx, rax
	lea	rcx, OFFSET FLAT:$SG4055
	call	SeTextOut
$LN13@page_fault:
$LN14@page_fault:
$LN16@page_fault:
$skip$23:

; 210  : 		}
; 211  : 	}
; 212  : 	
; 213  : skip:
; 214  : 	panic("Page Fault !! \r\n");

	lea	rcx, OFFSET FLAT:$SG4056
	call	?panic@@YAXPEBDZZ			; panic

; 215  : 	uint64_t vaddr_ = (uint64_t)vaddr;

	mov	rax, QWORD PTR vaddr$[rsp]
	mov	QWORD PTR vaddr_$[rsp], rax

; 216  : 	uint64_t vaddr_aligned = VIRT_ADDR_ALIGN(vaddr_);

	mov	rax, QWORD PTR vaddr_$[rsp]
	and	rax, -4096				; fffffffffffff000H
	mov	QWORD PTR vaddr_aligned$[rsp], rax

; 217  : 	bool _mapped = false;

	mov	BYTE PTR _mapped$[rsp], 0

; 218  : 	if (present) {

	cmp	DWORD PTR present$[rsp], 0
	je	SHORT $LN12@page_fault

; 219  : 		SeTextOut("Page Not Present \r\n");

	lea	rcx, OFFSET FLAT:$SG4062
	call	SeTextOut
	jmp	$LN11@page_fault
$LN12@page_fault:

; 220  : 	}
; 221  : 	else if (rw) {

	cmp	DWORD PTR rw$[rsp], 0
	je	$LN10@page_fault

; 222  : 		SeTextOut("Read/Write \r\n");

	lea	rcx, OFFSET FLAT:$SG4065
	call	SeTextOut

; 223  : 		void* phys = AuGetPhysicalAddress(vaddr_aligned);

	mov	rcx, QWORD PTR vaddr_aligned$[rsp]
	call	AuGetPhysicalAddress
	mov	QWORD PTR phys$4[rsp], rax

; 224  : 		AuVPage *vpage = AuVmmngrGetPage(vaddr_aligned, 0, 0);

	xor	r8d, r8d
	xor	edx, edx
	mov	rcx, QWORD PTR vaddr_aligned$[rsp]
	call	AuVmmngrGetPage
	mov	QWORD PTR vpage$2[rsp], rax

; 225  : 		SeTextOut("VPage rw -> %d , user -> %d \r\n", vpage->bits.writable, vpage->bits.user);

	mov	rax, QWORD PTR vpage$2[rsp]
	mov	rax, QWORD PTR [rax]
	shr	rax, 2
	and	rax, 1
	mov	rcx, QWORD PTR vpage$2[rsp]
	mov	rcx, QWORD PTR [rcx]
	shr	rcx, 1
	and	rcx, 1
	mov	r8, rax
	mov	rdx, rcx
	lea	rcx, OFFSET FLAT:$SG4068
	call	SeTextOut

; 226  : 		SeTextOut("VPage phys1-> %x, phys2-> %x \r\n", phys, (vpage->bits.page << PAGE_SHIFT));

	mov	rax, QWORD PTR vpage$2[rsp]
	mov	rax, QWORD PTR [rax]
	shr	rax, 12
	and	rax, 268435455				; 0fffffffH
	shl	rax, 12
	mov	r8, rax
	mov	rdx, QWORD PTR phys$4[rsp]
	lea	rcx, OFFSET FLAT:$SG4069
	call	SeTextOut
	jmp	SHORT $LN9@page_fault
$LN10@page_fault:

; 227  : 	}
; 228  : 	else if (us)

	cmp	DWORD PTR us$[rsp], 0
	je	SHORT $LN8@page_fault

; 229  : 		SeTextOut("User bit not set \r\n");

	lea	rcx, OFFSET FLAT:$SG4072
	call	SeTextOut
	jmp	SHORT $LN7@page_fault
$LN8@page_fault:

; 230  : 	else if (resv)

	cmp	DWORD PTR resv$[rsp], 0
	je	SHORT $LN6@page_fault

; 231  : 		SeTextOut("Reserved page \r\n");

	lea	rcx, OFFSET FLAT:$SG4075
	call	SeTextOut
	jmp	SHORT $LN5@page_fault
$LN6@page_fault:

; 232  : 	else if (id)

	cmp	DWORD PTR id$[rsp], 0
	je	SHORT $LN4@page_fault

; 233  : 		SeTextOut("Invalid page \r\n");

	lea	rcx, OFFSET FLAT:$SG4078
	call	SeTextOut
$LN4@page_fault:
$LN5@page_fault:
$LN7@page_fault:
$LN9@page_fault:
$LN11@page_fault:

; 234  : 
; 235  : 	SeTextOut("Virtual Address -> %x \r\n", vaddr_);

	mov	rdx, QWORD PTR vaddr_$[rsp]
	lea	rcx, OFFSET FLAT:$SG4079
	call	SeTextOut

; 236  : 	SeTextOut("Virtual Address aligned -> %x \r\n", vaddr_aligned);

	mov	rdx, QWORD PTR vaddr_aligned$[rsp]
	lea	rcx, OFFSET FLAT:$SG4080
	call	SeTextOut

; 237  : 	SeTextOut("RSP -> %x \r\n", frame->rsp);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+40]
	lea	rcx, OFFSET FLAT:$SG4081
	call	SeTextOut

; 238  : 	SeTextOut("RIP -> %x \r\n", frame->rip);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+16]
	lea	rcx, OFFSET FLAT:$SG4082
	call	SeTextOut

; 239  : 	SeTextOut("CS -> %x, SS -> %x \r\n", frame->cs, frame->ss);

	mov	rax, QWORD PTR frame$[rsp]
	mov	r8, QWORD PTR [rax+48]
	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+24]
	lea	rcx, OFFSET FLAT:$SG4083
	call	SeTextOut

; 240  : 	if (!_mapped) {

	movzx	eax, BYTE PTR _mapped$[rsp]
	test	eax, eax
	jne	SHORT $LN3@page_fault
$LN2@page_fault:

; 241  : 		for (;;);

	jmp	SHORT $LN2@page_fault
$LN3@page_fault:
$LN18@page_fault:

; 242  : 	}
; 243  : }

	add	rsp, 168				; 000000a8H
	ret	0
?page_fault@@YAX_KPEAX@Z ENDP				; page_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
frame$ = 32
tv82 = 40
v$ = 64
p$ = 72
?general_protection_fault@@YAX_KPEAX@Z PROC		; general_protection_fault

; 156  : void general_protection_fault(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 157  : 	x64_cli();

	call	x64_cli

; 158  : 	interrupt_stack_frame *frame = (interrupt_stack_frame*)p;

	mov	rax, QWORD PTR p$[rsp]
	mov	QWORD PTR frame$[rsp], rax

; 159  : 	panic("Genral Protection Fault \r\n");

	lea	rcx, OFFSET FLAT:$SG4014
	call	?panic@@YAXPEBDZZ			; panic

; 160  : 	SeTextOut("General Protection Fault \r\n");

	lea	rcx, OFFSET FLAT:$SG4015
	call	SeTextOut

; 161  : 	SeTextOut("__PROCESSOR TRACE__ \r\n");

	lea	rcx, OFFSET FLAT:$SG4016
	call	SeTextOut

; 162  : 	SeTextOut("RIP -> %x \r\n", frame->rip);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+16]
	lea	rcx, OFFSET FLAT:$SG4017
	call	SeTextOut

; 163  : 	SeTextOut("Stack -> %x \r\n", frame->rsp);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+40]
	lea	rcx, OFFSET FLAT:$SG4018
	call	SeTextOut

; 164  : 	SeTextOut("RFLAGS -> %x \r\n", frame->rflags);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+32]
	lea	rcx, OFFSET FLAT:$SG4019
	call	SeTextOut

; 165  : 	SeTextOut("CS -> %x, SS -> %x \r\n", frame->cs, frame->ss);

	mov	rax, QWORD PTR frame$[rsp]
	mov	r8, QWORD PTR [rax+48]
	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+24]
	lea	rcx, OFFSET FLAT:$SG4020
	call	SeTextOut

; 166  : 	SeTextOut("Current thread ->id %d , %s\r\n", AuGetCurrentThread()->id, AuGetCurrentThread()->name);

	call	AuGetCurrentThread
	add	rax, 284				; 0000011cH
	mov	QWORD PTR tv82[rsp], rax
	call	AuGetCurrentThread
	movzx	eax, WORD PTR [rax+301]
	mov	rcx, QWORD PTR tv82[rsp]
	mov	r8, rcx
	mov	edx, eax
	lea	rcx, OFFSET FLAT:$SG4021
	call	SeTextOut
$LN2@general_pr:

; 167  : 	for (;;);

	jmp	SHORT $LN2@general_pr

; 168  : }

	add	rsp, 56					; 00000038H
	ret	0
?general_protection_fault@@YAX_KPEAX@Z ENDP		; general_protection_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
v$ = 48
p$ = 56
?stack_fault@@YAX_KPEAX@Z PROC				; stack_fault

; 148  : void stack_fault(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 149  : 	x64_cli();

	call	x64_cli

; 150  : 	panic("\nStack Fault at ");

	lea	rcx, OFFSET FLAT:$SG4004
	call	?panic@@YAXPEBDZZ			; panic
$LN2@stack_faul:

; 151  : 	for (;;);

	jmp	SHORT $LN2@stack_faul

; 152  : }

	add	rsp, 40					; 00000028H
	ret	0
?stack_fault@@YAX_KPEAX@Z ENDP				; stack_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
v$ = 48
p$ = 56
?no_segment_fault@@YAX_KPEAX@Z PROC			; no_segment_fault

; 141  : void no_segment_fault(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 142  : 	x64_cli();

	call	x64_cli

; 143  : 	panic("\nNo Segment Fault");

	lea	rcx, OFFSET FLAT:$SG3996
	call	?panic@@YAXPEBDZZ			; panic
$LN2@no_segment:

; 144  : 	for (;;);

	jmp	SHORT $LN2@no_segment

; 145  : }

	add	rsp, 40					; 00000028H
	ret	0
?no_segment_fault@@YAX_KPEAX@Z ENDP			; no_segment_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
v$ = 48
p$ = 56
?invalid_tss_fault@@YAX_KPEAX@Z PROC			; invalid_tss_fault

; 134  : void invalid_tss_fault(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 135  : 	x64_cli();

	call	x64_cli

; 136  : 	panic("\nInvalid TSS Fault ");

	lea	rcx, OFFSET FLAT:$SG3988
	call	?panic@@YAXPEBDZZ			; panic
$LN2@invalid_ts:

; 137  : 	for (;;);

	jmp	SHORT $LN2@invalid_ts

; 138  : }

	add	rsp, 40					; 00000028H
	ret	0
?invalid_tss_fault@@YAX_KPEAX@Z ENDP			; invalid_tss_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
v$ = 48
p$ = 56
?double_fault_abort@@YAX_KPEAX@Z PROC			; double_fault_abort

; 127  : void double_fault_abort(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 128  : 	x64_cli();

	call	x64_cli

; 129  : 	panic("\nDouble Fault Abort");

	lea	rcx, OFFSET FLAT:$SG3980
	call	?panic@@YAXPEBDZZ			; panic
$LN2@double_fau:

; 130  : 	for (;;);

	jmp	SHORT $LN2@double_fau

; 131  : }

	add	rsp, 40					; 00000028H
	ret	0
?double_fault_abort@@YAX_KPEAX@Z ENDP			; double_fault_abort
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
v$ = 48
p$ = 56
?no_device_fault@@YAX_KPEAX@Z PROC			; no_device_fault

; 120  : void no_device_fault(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 121  : 	x64_cli();

	call	x64_cli

; 122  : 	panic("\nNo Device Fault");

	lea	rcx, OFFSET FLAT:$SG3972
	call	?panic@@YAXPEBDZZ			; panic
$LN2@no_device_:

; 123  : 	for (;;);

	jmp	SHORT $LN2@no_device_

; 124  : }

	add	rsp, 40					; 00000028H
	ret	0
?no_device_fault@@YAX_KPEAX@Z ENDP			; no_device_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
frame$ = 32
v$ = 64
p$ = 72
?invalid_opcode_fault@@YAX_KPEAX@Z PROC			; invalid_opcode_fault

; 105  : void invalid_opcode_fault(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 106  : 	x64_cli();

	call	x64_cli

; 107  : 	interrupt_stack_frame *frame = (interrupt_stack_frame*)p;

	mov	rax, QWORD PTR p$[rsp]
	mov	QWORD PTR frame$[rsp], rax

; 108  : 	panic("Invalid Opcode Fault \r\n");

	lea	rcx, OFFSET FLAT:$SG3957
	call	?panic@@YAXPEBDZZ			; panic

; 109  : 	SeTextOut("Invalid Opcode Fault \r\n");

	lea	rcx, OFFSET FLAT:$SG3958
	call	SeTextOut

; 110  : 	SeTextOut("__PROCESSOR TRACE__ \r\n");

	lea	rcx, OFFSET FLAT:$SG3959
	call	SeTextOut

; 111  : 	SeTextOut("RIP -> %x\n", frame->rip);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+16]
	lea	rcx, OFFSET FLAT:$SG3960
	call	SeTextOut

; 112  : 	SeTextOut("Stack -> %x\n", frame->rsp);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+40]
	lea	rcx, OFFSET FLAT:$SG3961
	call	SeTextOut

; 113  : 	SeTextOut("RFLAGS -> %x\n", frame->rflags);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+32]
	lea	rcx, OFFSET FLAT:$SG3962
	call	SeTextOut

; 114  : 	SeTextOut("CS -> %x\n", frame->cs);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+24]
	lea	rcx, OFFSET FLAT:$SG3963
	call	SeTextOut

; 115  : 	SeTextOut("SS -> %x\n", frame->ss);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+48]
	lea	rcx, OFFSET FLAT:$SG3964
	call	SeTextOut
$LN2@invalid_op:

; 116  : 	for (;;);

	jmp	SHORT $LN2@invalid_op

; 117  : }

	add	rsp, 56					; 00000038H
	ret	0
?invalid_opcode_fault@@YAX_KPEAX@Z ENDP			; invalid_opcode_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
v$ = 48
p$ = 56
?bounds_check_fault@@YAX_KPEAX@Z PROC			; bounds_check_fault

; 98   : void bounds_check_fault(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 99   : 	x64_cli();

	call	x64_cli

; 100  : 	panic("\nBound Check Fault");

	lea	rcx, OFFSET FLAT:$SG3947
	call	?panic@@YAXPEBDZZ			; panic
$LN2@bounds_che:

; 101  : 	for (;;);

	jmp	SHORT $LN2@bounds_che

; 102  : }

	add	rsp, 40					; 00000028H
	ret	0
?bounds_check_fault@@YAX_KPEAX@Z ENDP			; bounds_check_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
v$ = 48
p$ = 56
?overflow_trap@@YAX_KPEAX@Z PROC			; overflow_trap

; 91   : void overflow_trap(size_t v, void* p){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 92   : 	x64_cli();

	call	x64_cli

; 93   : 	panic("\nOverflow Trap");

	lea	rcx, OFFSET FLAT:$SG3939
	call	?panic@@YAXPEBDZZ			; panic
$LN2@overflow_t:

; 94   : 	for (;;);

	jmp	SHORT $LN2@overflow_t

; 95   : }

	add	rsp, 40					; 00000028H
	ret	0
?overflow_trap@@YAX_KPEAX@Z ENDP			; overflow_trap
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
vector$ = 48
param$ = 56
?breakpoint_trap@@YAX_KPEAX@Z PROC			; breakpoint_trap

; 84   : void breakpoint_trap(size_t vector, void* param){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 85   : 	x64_cli();

	call	x64_cli

; 86   : 	panic("\nBreakpoint Trap");

	lea	rcx, OFFSET FLAT:$SG3931
	call	?panic@@YAXPEBDZZ			; panic
$LN2@breakpoint:

; 87   : 	for (;;);

	jmp	SHORT $LN2@breakpoint

; 88   : }

	add	rsp, 40					; 00000028H
	ret	0
?breakpoint_trap@@YAX_KPEAX@Z ENDP			; breakpoint_trap
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
vector$ = 48
param$ = 56
?nmi_trap@@YAX_KPEAX@Z PROC				; nmi_trap

; 76   : void nmi_trap(size_t vector, void* param){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 77   : 	x64_cli();

	call	x64_cli

; 78   : 	panic("\nNMI [Non-Muskable-Interrupt] Trap");

	lea	rcx, OFFSET FLAT:$SG3923
	call	?panic@@YAXPEBDZZ			; panic
$LN2@nmi_trap:

; 79   : 	for (;;);

	jmp	SHORT $LN2@nmi_trap

; 80   : 
; 81   : }

	add	rsp, 40					; 00000028H
	ret	0
?nmi_trap@@YAX_KPEAX@Z ENDP				; nmi_trap
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
frame$ = 32
vector$ = 64
param$ = 72
?single_step_trap@@YAX_KPEAX@Z PROC			; single_step_trap

; 69   : void single_step_trap(size_t vector, void* param) {

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 70   : 	x64_cli();

	call	x64_cli

; 71   : 	interrupt_stack_frame *frame = (interrupt_stack_frame*)param;

	mov	rax, QWORD PTR param$[rsp]
	mov	QWORD PTR frame$[rsp], rax

; 72   : 	panic("\nSingle Step Trap");

	lea	rcx, OFFSET FLAT:$SG3915
	call	?panic@@YAXPEBDZZ			; panic
$LN2@single_ste:

; 73   : 	for (;;);

	jmp	SHORT $LN2@single_ste

; 74   : }

	add	rsp, 56					; 00000038H
	ret	0
?single_step_trap@@YAX_KPEAX@Z ENDP			; single_step_trap
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
frame$ = 32
vector$ = 64
param$ = 72
?divide_by_zero_fault@@YAX_KPEAX@Z PROC			; divide_by_zero_fault

; 56   : void divide_by_zero_fault(size_t vector, void* param) {

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 57   : 	x64_cli();

	call	x64_cli

; 58   : 	interrupt_stack_frame *frame = (interrupt_stack_frame*)param;

	mov	rax, QWORD PTR param$[rsp]
	mov	QWORD PTR frame$[rsp], rax

; 59   : 	panic("\nDivide by 0");

	lea	rcx, OFFSET FLAT:$SG3900
	call	?panic@@YAXPEBDZZ			; panic

; 60   : 	SeTextOut("Divide by 0 \r\n");

	lea	rcx, OFFSET FLAT:$SG3901
	call	SeTextOut

; 61   : 	SeTextOut("__PROCESSOR_DATA__ \r\n");

	lea	rcx, OFFSET FLAT:$SG3902
	call	SeTextOut

; 62   : 	SeTextOut("RIP -> %x \r\n", frame->rip);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+16]
	lea	rcx, OFFSET FLAT:$SG3903
	call	SeTextOut

; 63   : 	SeTextOut("RSP -> %x \r\n", frame->rsp);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+40]
	lea	rcx, OFFSET FLAT:$SG3904
	call	SeTextOut

; 64   : 	SeTextOut("RFLAGS -> %x \r\n", frame->rflags);

	mov	rax, QWORD PTR frame$[rsp]
	mov	rdx, QWORD PTR [rax+32]
	lea	rcx, OFFSET FLAT:$SG3905
	call	SeTextOut
$LN2@divide_by_:

; 65   : 
; 66   : 	for (;;);

	jmp	SHORT $LN2@divide_by_

; 67   : }

	add	rsp, 56					; 00000038H
	ret	0
?divide_by_zero_fault@@YAX_KPEAX@Z ENDP			; divide_by_zero_fault
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
msg$ = 48
?panic@@YAXPEBDZZ PROC					; panic

; 47   : void panic(const char* msg, ...) {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+32], r9
	sub	rsp, 40					; 00000028H

; 48   : 	SeTextOut("\r\n ***ARCH x86_64 : Kernel Panic!!! *** \r\n");

	lea	rcx, OFFSET FLAT:$SG3888
	call	SeTextOut

; 49   : 	SeTextOut("[Aurora Kernel]: We are sorry to say that, a processor invalid exception has occured \r\n");

	lea	rcx, OFFSET FLAT:$SG3889
	call	SeTextOut

; 50   : 	SeTextOut("[Aurora Kernel]: please inform it to the master of the kernel \r\n");

	lea	rcx, OFFSET FLAT:$SG3890
	call	SeTextOut

; 51   : 	SeTextOut("[Aurora Kernel]: Below is the code of exception \r\n");

	lea	rcx, OFFSET FLAT:$SG3891
	call	SeTextOut

; 52   : 	SeTextOut("[Aurora Kernel]: Current Processor id -> %d \r\n", AuPerCPUGetCpuID());

	call	?AuPerCPUGetCpuID@@YAEXZ		; AuPerCPUGetCpuID
	movzx	eax, al
	mov	edx, eax
	lea	rcx, OFFSET FLAT:$SG3892
	call	SeTextOut

; 53   : 	SeTextOut(" %s \r\n", msg);

	mov	rdx, QWORD PTR msg$[rsp]
	lea	rcx, OFFSET FLAT:$SG3893
	call	SeTextOut

; 54   : }

	add	rsp, 40					; 00000028H
	ret	0
?panic@@YAXPEBDZZ ENDP					; panic
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_exception.cpp
_TEXT	SEGMENT
?x86_64_exception_init@@YAXXZ PROC			; x86_64_exception_init

; 285  : void x86_64_exception_init() {

$LN3:
	sub	rsp, 40					; 00000028H

; 286  : 	setvect(0, divide_by_zero_fault);

	lea	rdx, OFFSET FLAT:?divide_by_zero_fault@@YAX_KPEAX@Z ; divide_by_zero_fault
	xor	ecx, ecx
	call	setvect

; 287  : 	setvect(1, single_step_trap);

	lea	rdx, OFFSET FLAT:?single_step_trap@@YAX_KPEAX@Z ; single_step_trap
	mov	ecx, 1
	call	setvect

; 288  : 	setvect(2, nmi_trap);

	lea	rdx, OFFSET FLAT:?nmi_trap@@YAX_KPEAX@Z	; nmi_trap
	mov	ecx, 2
	call	setvect

; 289  : 	setvect(3, breakpoint_trap);

	lea	rdx, OFFSET FLAT:?breakpoint_trap@@YAX_KPEAX@Z ; breakpoint_trap
	mov	ecx, 3
	call	setvect

; 290  : 	setvect(4, overflow_trap);

	lea	rdx, OFFSET FLAT:?overflow_trap@@YAX_KPEAX@Z ; overflow_trap
	mov	ecx, 4
	call	setvect

; 291  : 	setvect(5, bounds_check_fault);

	lea	rdx, OFFSET FLAT:?bounds_check_fault@@YAX_KPEAX@Z ; bounds_check_fault
	mov	ecx, 5
	call	setvect

; 292  : 	setvect(6, invalid_opcode_fault);

	lea	rdx, OFFSET FLAT:?invalid_opcode_fault@@YAX_KPEAX@Z ; invalid_opcode_fault
	mov	ecx, 6
	call	setvect

; 293  : 	setvect(7, no_device_fault);

	lea	rdx, OFFSET FLAT:?no_device_fault@@YAX_KPEAX@Z ; no_device_fault
	mov	ecx, 7
	call	setvect

; 294  : 	setvect(8, double_fault_abort);

	lea	rdx, OFFSET FLAT:?double_fault_abort@@YAX_KPEAX@Z ; double_fault_abort
	mov	ecx, 8
	call	setvect

; 295  : 	setvect(10, invalid_tss_fault);

	lea	rdx, OFFSET FLAT:?invalid_tss_fault@@YAX_KPEAX@Z ; invalid_tss_fault
	mov	ecx, 10
	call	setvect

; 296  : 	setvect(11, no_segment_fault);

	lea	rdx, OFFSET FLAT:?no_segment_fault@@YAX_KPEAX@Z ; no_segment_fault
	mov	ecx, 11
	call	setvect

; 297  : 	setvect(12, stack_fault);

	lea	rdx, OFFSET FLAT:?stack_fault@@YAX_KPEAX@Z ; stack_fault
	mov	ecx, 12
	call	setvect

; 298  : 	setvect(13, general_protection_fault);

	lea	rdx, OFFSET FLAT:?general_protection_fault@@YAX_KPEAX@Z ; general_protection_fault
	mov	ecx, 13
	call	setvect

; 299  : 	setvect(14, page_fault);

	lea	rdx, OFFSET FLAT:?page_fault@@YAX_KPEAX@Z ; page_fault
	mov	ecx, 14
	call	setvect

; 300  : 	setvect(16, fpu_fault);

	lea	rdx, OFFSET FLAT:?fpu_fault@@YAX_KPEAX@Z ; fpu_fault
	mov	ecx, 16
	call	setvect

; 301  : 	setvect(17, alignment_check_fault);

	lea	rdx, OFFSET FLAT:?alignment_check_fault@@YAX_KPEAX@Z ; alignment_check_fault
	mov	ecx, 17
	call	setvect

; 302  : 	setvect(18, machine_check_abort);

	lea	rdx, OFFSET FLAT:?machine_check_abort@@YAX_KPEAX@Z ; machine_check_abort
	mov	ecx, 18
	call	setvect

; 303  : 	setvect(19, simd_fpu_fault);

	lea	rdx, OFFSET FLAT:?simd_fpu_fault@@YAX_KPEAX@Z ; simd_fpu_fault
	mov	ecx, 19
	call	setvect

; 304  : }

	add	rsp, 40					; 00000028H
	ret	0
?x86_64_exception_init@@YAXXZ ENDP			; x86_64_exception_init
_TEXT	ENDS
END