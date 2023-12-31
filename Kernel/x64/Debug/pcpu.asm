; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?cpus@@3PAPEAU_cpu_@@A				; cpus
_BSS	SEGMENT
?cpus@@3PAPEAU_cpu_@@A DQ 0100H DUP (?)			; cpus
_BSS	ENDS
PUBLIC	?AuCreatePerCPU@@YAXPEAX@Z			; AuCreatePerCPU
PUBLIC	?AuGetPerCPU@@YAPEAU_cpu_@@E@Z			; AuGetPerCPU
PUBLIC	?AuPerCPUSetCpuID@@YAXE@Z			; AuPerCPUSetCpuID
PUBLIC	?AuPerCPUGetCpuID@@YAEXZ			; AuPerCPUGetCpuID
PUBLIC	?AuPerCPUSetCurrentThread@@YAXPEAX@Z		; AuPerCPUSetCurrentThread
PUBLIC	?AuPerCPUGetCurrentThread@@YAPEAU_au_thread_@@XZ ; AuPerCPUGetCurrentThread
PUBLIC	?AuPerCPUSetKernelTSS@@YAXPEAU_tss@@@Z		; AuPerCPUSetKernelTSS
PUBLIC	?AuPerCPUGetKernelTSS@@YAPEAU_tss@@XZ		; AuPerCPUGetKernelTSS
EXTRN	x64_write_msr:PROC
EXTRN	x64_gs_readb:PROC
EXTRN	x64_gs_readq:PROC
EXTRN	x64_gs_writeb:PROC
EXTRN	x64_gs_writeq:PROC
EXTRN	kmalloc:PROC
EXTRN	memset:PROC
pdata	SEGMENT
$pdata$?AuCreatePerCPU@@YAXPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+106
	DD	imagerel $unwind$?AuCreatePerCPU@@YAXPEAX@Z
$pdata$?AuPerCPUSetCpuID@@YAXE@Z DD imagerel $LN3
	DD	imagerel $LN3+25
	DD	imagerel $unwind$?AuPerCPUSetCpuID@@YAXE@Z
$pdata$?AuPerCPUGetCpuID@@YAEXZ DD imagerel $LN3
	DD	imagerel $LN3+16
	DD	imagerel $unwind$?AuPerCPUGetCpuID@@YAEXZ
$pdata$?AuPerCPUSetCurrentThread@@YAXPEAX@Z DD imagerel $LN3
	DD	imagerel $LN3+29
	DD	imagerel $unwind$?AuPerCPUSetCurrentThread@@YAXPEAX@Z
$pdata$?AuPerCPUGetCurrentThread@@YAPEAU_au_thread_@@XZ DD imagerel $LN3
	DD	imagerel $LN3+19
	DD	imagerel $unwind$?AuPerCPUGetCurrentThread@@YAPEAU_au_thread_@@XZ
$pdata$?AuPerCPUSetKernelTSS@@YAXPEAU_tss@@@Z DD imagerel $LN3
	DD	imagerel $LN3+29
	DD	imagerel $unwind$?AuPerCPUSetKernelTSS@@YAXPEAU_tss@@@Z
$pdata$?AuPerCPUGetKernelTSS@@YAPEAU_tss@@XZ DD imagerel $LN3
	DD	imagerel $LN3+19
	DD	imagerel $unwind$?AuPerCPUGetKernelTSS@@YAPEAU_tss@@XZ
pdata	ENDS
xdata	SEGMENT
$unwind$?AuCreatePerCPU@@YAXPEAX@Z DD 010901H
	DD	06209H
$unwind$?AuPerCPUSetCpuID@@YAXE@Z DD 010801H
	DD	04208H
$unwind$?AuPerCPUGetCpuID@@YAEXZ DD 010401H
	DD	04204H
$unwind$?AuPerCPUSetCurrentThread@@YAXPEAX@Z DD 010901H
	DD	04209H
$unwind$?AuPerCPUGetCurrentThread@@YAPEAU_au_thread_@@XZ DD 010401H
	DD	04204H
$unwind$?AuPerCPUSetKernelTSS@@YAXPEAU_tss@@@Z DD 010901H
	DD	04209H
$unwind$?AuPerCPUGetKernelTSS@@YAPEAU_tss@@XZ DD 010401H
	DD	04204H
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\pcpu.cpp
_TEXT	SEGMENT
?AuPerCPUGetKernelTSS@@YAPEAU_tss@@XZ PROC		; AuPerCPUGetKernelTSS

; 108  : TSS* AuPerCPUGetKernelTSS() {

$LN3:
	sub	rsp, 40					; 00000028H

; 109  : 	return (TSS*)x64_gs_readq(9);

	mov	ecx, 9
	call	x64_gs_readq

; 110  : }

	add	rsp, 40					; 00000028H
	ret	0
?AuPerCPUGetKernelTSS@@YAPEAU_tss@@XZ ENDP		; AuPerCPUGetKernelTSS
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\pcpu.cpp
_TEXT	SEGMENT
tss$ = 48
?AuPerCPUSetKernelTSS@@YAXPEAU_tss@@@Z PROC		; AuPerCPUSetKernelTSS

; 101  : void AuPerCPUSetKernelTSS(TSS *tss){

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 102  : 	x64_gs_writeq(9, (uint64_t)tss);

	mov	rdx, QWORD PTR tss$[rsp]
	mov	ecx, 9
	call	x64_gs_writeq

; 103  : }

	add	rsp, 40					; 00000028H
	ret	0
?AuPerCPUSetKernelTSS@@YAXPEAU_tss@@@Z ENDP		; AuPerCPUSetKernelTSS
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\pcpu.cpp
_TEXT	SEGMENT
?AuPerCPUGetCurrentThread@@YAPEAU_au_thread_@@XZ PROC	; AuPerCPUGetCurrentThread

; 93   : AuThread* AuPerCPUGetCurrentThread() {

$LN3:
	sub	rsp, 40					; 00000028H

; 94   : 	return (AuThread*)x64_gs_readq(1);

	mov	ecx, 1
	call	x64_gs_readq

; 95   : }

	add	rsp, 40					; 00000028H
	ret	0
?AuPerCPUGetCurrentThread@@YAPEAU_au_thread_@@XZ ENDP	; AuPerCPUGetCurrentThread
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\pcpu.cpp
_TEXT	SEGMENT
thread$ = 48
?AuPerCPUSetCurrentThread@@YAXPEAX@Z PROC		; AuPerCPUSetCurrentThread

; 85   : void AuPerCPUSetCurrentThread(void* thread) {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 86   : 	x64_gs_writeq(1, (size_t)thread);

	mov	rdx, QWORD PTR thread$[rsp]
	mov	ecx, 1
	call	x64_gs_writeq

; 87   : }

	add	rsp, 40					; 00000028H
	ret	0
?AuPerCPUSetCurrentThread@@YAXPEAX@Z ENDP		; AuPerCPUSetCurrentThread
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\pcpu.cpp
_TEXT	SEGMENT
?AuPerCPUGetCpuID@@YAEXZ PROC				; AuPerCPUGetCpuID

; 76   : uint8_t AuPerCPUGetCpuID() {

$LN3:
	sub	rsp, 40					; 00000028H

; 77   : 	return x64_gs_readb(0);

	xor	ecx, ecx
	call	x64_gs_readb

; 78   : }

	add	rsp, 40					; 00000028H
	ret	0
?AuPerCPUGetCpuID@@YAEXZ ENDP				; AuPerCPUGetCpuID
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\pcpu.cpp
_TEXT	SEGMENT
id$ = 48
?AuPerCPUSetCpuID@@YAXE@Z PROC				; AuPerCPUSetCpuID

; 68   : void AuPerCPUSetCpuID(uint8_t id) {

$LN3:
	mov	BYTE PTR [rsp+8], cl
	sub	rsp, 40					; 00000028H

; 69   : 	x64_gs_writeb(0, id);

	movzx	edx, BYTE PTR id$[rsp]
	xor	ecx, ecx
	call	x64_gs_writeb

; 70   : }

	add	rsp, 40					; 00000028H
	ret	0
?AuPerCPUSetCpuID@@YAXE@Z ENDP				; AuPerCPUSetCpuID
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\pcpu.cpp
_TEXT	SEGMENT
id$ = 8
?AuGetPerCPU@@YAPEAU_cpu_@@E@Z PROC			; AuGetPerCPU

; 60   : CPUStruc* AuGetPerCPU(uint8_t id) {

	mov	BYTE PTR [rsp+8], cl

; 61   : 	return cpus[id];

	movzx	eax, BYTE PTR id$[rsp]
	lea	rcx, OFFSET FLAT:?cpus@@3PAPEAU_cpu_@@A	; cpus
	mov	rax, QWORD PTR [rcx+rax*8]

; 62   : }

	ret	0
?AuGetPerCPU@@YAPEAU_cpu_@@E@Z ENDP			; AuGetPerCPU
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\pcpu.cpp
_TEXT	SEGMENT
cpu$ = 32
alloc$ = 64
?AuCreatePerCPU@@YAXPEAX@Z PROC				; AuCreatePerCPU

; 42   : void AuCreatePerCPU(void* alloc) {

$LN5:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 43   : 	CPUStruc *cpu;
; 44   : 	if (alloc != 0) 

	cmp	QWORD PTR alloc$[rsp], 0
	je	SHORT $LN2@AuCreatePe

; 45   : 		cpu = (CPUStruc*)alloc;

	mov	rax, QWORD PTR alloc$[rsp]
	mov	QWORD PTR cpu$[rsp], rax

; 46   : 	else {

	jmp	SHORT $LN1@AuCreatePe
$LN2@AuCreatePe:

; 47   : 		cpu = (CPUStruc*)kmalloc(sizeof(CPUStruc));

	mov	ecx, 17
	call	kmalloc
	mov	QWORD PTR cpu$[rsp], rax

; 48   : 		memset(cpu, 0, sizeof(CPUStruc));

	mov	r8d, 17
	xor	edx, edx
	mov	rcx, QWORD PTR cpu$[rsp]
	call	memset
$LN1@AuCreatePe:

; 49   : 	}
; 50   : 
; 51   : 	cpus[cpu->cpu_id] = cpu;

	mov	rax, QWORD PTR cpu$[rsp]
	movzx	eax, BYTE PTR [rax]
	lea	rcx, OFFSET FLAT:?cpus@@3PAPEAU_cpu_@@A	; cpus
	mov	rdx, QWORD PTR cpu$[rsp]
	mov	QWORD PTR [rcx+rax*8], rdx

; 52   : 	x64_write_msr(MSR_IA32_GS_BASE, (size_t)cpu);

	mov	rdx, QWORD PTR cpu$[rsp]
	mov	ecx, -1073741567			; c0000101H
	call	x64_write_msr

; 53   : }

	add	rsp, 56					; 00000038H
	ret	0
?AuCreatePerCPU@@YAXPEAX@Z ENDP				; AuCreatePerCPU
_TEXT	ENDS
END
