; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

CONST	SEGMENT
$SG4098	DB	'Used RAM -> %d GB  Avail -> %d GB ', 0dH, 0aH, 00H
	ORG $+3
$SG4099	DB	'Process cleaned ', 0dH, 0aH, 00H
	ORG $+5
$SG4057	DB	'KStack freed %s ', 0dH, 0aH, 00H
	ORG $+5
$SG4059	DB	'T->Uentry -> %x ', 0dH, 0aH, 00H
	ORG $+5
$SG4080	DB	'cleaning thread -> %x %s', 0aH, 00H
	ORG $+6
$SG4085	DB	'Stack_ -> %x stack -> %x', 0dH, 0aH, 00H
	ORG $+5
$SG4087	DB	'Thread user stack freed ', 0dH, 0aH, 00H
	ORG $+5
$SG4088	DB	'Thread freed ', 0dH, 0aH, 00H
$SG4090	DB	'Uenty -> %x ', 0dH, 0aH, 00H
CONST	ENDS
PUBLIC	?AuProcessClean@@YAXPEAU_au_proc_@@0@Z		; AuProcessClean
PUBLIC	?FreeUserStack@@YAXPEA_KPEAX@Z			; FreeUserStack
PUBLIC	?FreeImage@@YAXPEAU_au_proc_@@@Z		; FreeImage
PUBLIC	?AuThreadFree@@YAXPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuThreadFree
EXTRN	list_remove:PROC
EXTRN	?AuThreadCleanTrash@@YAXPEAU_au_thread_@@@Z:PROC ; AuThreadCleanTrash
EXTRN	?AuRemoveProcess@@YAXPEAU_au_proc_@@0@Z:PROC	; AuRemoveProcess
EXTRN	?KernelStackFree@@YAXPEAU_au_proc_@@PEAXPEA_K@Z:PROC ; KernelStackFree
EXTRN	AuGetPhysicalAddressEx:PROC
EXTRN	AuPmmngrFree:PROC
EXTRN	P2V:PROC
EXTRN	V2P:PROC
EXTRN	?AuPmmngrGetFreeMem@@YA_KXZ:PROC		; AuPmmngrGetFreeMem
EXTRN	?AuPmmngrGetTotalMem@@YA_KXZ:PROC		; AuPmmngrGetTotalMem
EXTRN	?AuRemoveVMArea@@YAXPEAU_au_proc_@@PEAU_vm_area_@@@Z:PROC ; AuRemoveVMArea
EXTRN	?AuVMAreaGet@@YAPEAU_vm_area_@@PEAU_au_proc_@@_K@Z:PROC ; AuVMAreaGet
EXTRN	kfree:PROC
EXTRN	SeTextOut:PROC
pdata	SEGMENT
$pdata$?AuProcessClean@@YAXPEAU_au_proc_@@0@Z DD imagerel $LN14
	DD	imagerel $LN14+808
	DD	imagerel $unwind$?AuProcessClean@@YAXPEAU_au_proc_@@0@Z
$pdata$?FreeUserStack@@YAXPEA_KPEAX@Z DD imagerel $LN7
	DD	imagerel $LN7+122
	DD	imagerel $unwind$?FreeUserStack@@YAXPEA_KPEAX@Z
$pdata$?FreeImage@@YAXPEAU_au_proc_@@@Z DD imagerel $LN7
	DD	imagerel $LN7+190
	DD	imagerel $unwind$?FreeImage@@YAXPEAU_au_proc_@@@Z
$pdata$?AuThreadFree@@YAXPEAU_au_proc_@@PEAU_au_thread_@@@Z DD imagerel $LN6
	DD	imagerel $LN6+278
	DD	imagerel $unwind$?AuThreadFree@@YAXPEAU_au_proc_@@PEAU_au_thread_@@@Z
pdata	ENDS
xdata	SEGMENT
$unwind$?AuProcessClean@@YAXPEAU_au_proc_@@0@Z DD 021101H
	DD	0110111H
$unwind$?FreeUserStack@@YAXPEA_KPEAX@Z DD 010e01H
	DD	0820eH
$unwind$?FreeImage@@YAXPEAU_au_proc_@@@Z DD 010901H
	DD	0a209H
$unwind$?AuThreadFree@@YAXPEAU_au_proc_@@PEAU_au_thread_@@@Z DD 010e01H
	DD	0820eH
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\clean.cpp
_TEXT	SEGMENT
k_stack$1 = 32
k_stack_$2 = 40
k_stack$3 = 48
k_stack_$4 = 56
proc$ = 80
t$ = 88
?AuThreadFree@@YAXPEAU_au_proc_@@PEAU_au_thread_@@@Z PROC ; AuThreadFree

; 82   : void AuThreadFree(AuProcess* proc,AuThread* t) {

$LN6:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 72					; 00000048H

; 83   : 	kfree(t->fx_state);

	mov	rax, QWORD PTR t$[rsp]
	mov	rcx, QWORD PTR [rax+272]
	call	kfree

; 84   : 	/* free up the kernel stack */
; 85   : 
; 86   : 	/* if the thread is main thread, the kernel
; 87   : 	 * stack is directly allocated over physical
; 88   : 	 * memory
; 89   : 	 */
; 90   : 	if (t->priviledge & THREAD_LEVEL_MAIN_THREAD){

	mov	rax, QWORD PTR t$[rsp]
	movzx	eax, BYTE PTR [rax+311]
	and	eax, 8
	test	eax, eax
	je	SHORT $LN3@AuThreadFr

; 91   : 		/* increase k_stack by 32 bytes because, 32 bytes
; 92   : 		 * being decresed at AuLoadExecToProc function 
; 93   : 		 */
; 94   : 		uint64_t k_stack = t->frame.kern_esp + 32;

	mov	rax, QWORD PTR t$[rsp]
	mov	rax, QWORD PTR [rax+200]
	add	rax, 32					; 00000020H
	mov	QWORD PTR k_stack$1[rsp], rax

; 95   : 		uint64_t k_stack_ = k_stack - PAGE_SIZE;

	mov	rax, QWORD PTR k_stack$1[rsp]
	sub	rax, 4096				; 00001000H
	mov	QWORD PTR k_stack_$2[rsp], rax

; 96   : 		AuPmmngrFree((void*)V2P((size_t)k_stack_));

	mov	rcx, QWORD PTR k_stack_$2[rsp]
	call	V2P
	mov	rcx, rax
	call	AuPmmngrFree
$LN3@AuThreadFr:

; 97   : 	}
; 98   : 	/* if its a sub thread, kstack is allocated over
; 99   : 	 * virtual memory with an index so free it
; 100  : 	 */
; 101  : 	if (t->priviledge & THREAD_LEVEL_SUBTHREAD) {

	mov	rax, QWORD PTR t$[rsp]
	movzx	eax, BYTE PTR [rax+311]
	and	eax, 4
	test	eax, eax
	je	SHORT $LN2@AuThreadFr

; 102  : 		uint64_t k_stack = t->frame.kern_esp;

	mov	rax, QWORD PTR t$[rsp]
	mov	rax, QWORD PTR [rax+200]
	mov	QWORD PTR k_stack$3[rsp], rax

; 103  : 		uint64_t k_stack_ = k_stack - 8192;

	mov	rax, QWORD PTR k_stack$3[rsp]
	sub	rax, 8192				; 00002000H
	mov	QWORD PTR k_stack_$4[rsp], rax

; 104  : 		KernelStackFree(proc, (void*)k_stack_, proc->cr3);

	mov	rax, QWORD PTR proc$[rsp]
	mov	r8, QWORD PTR [rax+24]
	mov	rdx, QWORD PTR k_stack_$4[rsp]
	mov	rcx, QWORD PTR proc$[rsp]
	call	?KernelStackFree@@YAXPEAU_au_proc_@@PEAXPEA_K@Z ; KernelStackFree

; 105  : 		SeTextOut("KStack freed %s \r\n", t->name);

	mov	rax, QWORD PTR t$[rsp]
	add	rax, 284				; 0000011cH
	mov	rdx, rax
	lea	rcx, OFFSET FLAT:$SG4057
	call	SeTextOut
$LN2@AuThreadFr:

; 106  : 		
; 107  : 	}
; 108  : 	if (t->uentry){

	mov	rax, QWORD PTR t$[rsp]
	cmp	QWORD PTR [rax+641], 0
	je	SHORT $LN1@AuThreadFr

; 109  : 		SeTextOut("T->Uentry -> %x \r\n", t->uentry);

	mov	rax, QWORD PTR t$[rsp]
	mov	rdx, QWORD PTR [rax+641]
	lea	rcx, OFFSET FLAT:$SG4059
	call	SeTextOut

; 110  : 		kfree(t->uentry);

	mov	rax, QWORD PTR t$[rsp]
	mov	rcx, QWORD PTR [rax+641]
	call	kfree
$LN1@AuThreadFr:

; 111  : 	}
; 112  : 	kfree(t);

	mov	rcx, QWORD PTR t$[rsp]
	call	kfree

; 113  : }

	add	rsp, 72					; 00000048H
	ret	0
?AuThreadFree@@YAXPEAU_au_proc_@@PEAU_au_thread_@@@Z ENDP ; AuThreadFree
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\clean.cpp
_TEXT	SEGMENT
i$1 = 32
physical_address$2 = 40
tv65 = 48
phys$3 = 56
image_area$ = 64
proc$ = 96
?FreeImage@@YAXPEAU_au_proc_@@@Z PROC			; FreeImage

; 64   : void FreeImage(AuProcess* proc) {

$LN7:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 88					; 00000058H

; 65   : 	/* Unmap the process image */
; 66   : 	for (uint32_t i = 0; i < proc->_image_size_ / 4096 + 1; i++) {

	mov	DWORD PTR i$1[rsp], 0
	jmp	SHORT $LN4@FreeImage
$LN3@FreeImage:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN4@FreeImage:
	mov	eax, DWORD PTR i$1[rsp]
	mov	QWORD PTR tv65[rsp], rax
	xor	edx, edx
	mov	rcx, QWORD PTR proc$[rsp]
	mov	rax, QWORD PTR [rcx+32]
	mov	ecx, 4096				; 00001000H
	div	rcx
	inc	rax
	mov	rcx, QWORD PTR tv65[rsp]
	cmp	rcx, rax
	jae	SHORT $LN2@FreeImage

; 67   : 		void* phys = AuGetPhysicalAddressEx(proc->cr3, proc->_image_base_ + i * 4096);

	imul	eax, DWORD PTR i$1[rsp], 4096		; 00001000H
	mov	eax, eax
	mov	rcx, QWORD PTR proc$[rsp]
	add	rax, QWORD PTR [rcx+40]
	mov	rdx, rax
	mov	rax, QWORD PTR proc$[rsp]
	mov	rcx, QWORD PTR [rax+24]
	call	AuGetPhysicalAddressEx
	mov	QWORD PTR phys$3[rsp], rax

; 68   : 		uint64_t physical_address = (uint64_t)V2P((uint64_t)phys);

	mov	rcx, QWORD PTR phys$3[rsp]
	call	V2P
	mov	QWORD PTR physical_address$2[rsp], rax

; 69   : 		if (physical_address != 0)

	cmp	QWORD PTR physical_address$2[rsp], 0
	je	SHORT $LN1@FreeImage

; 70   : 			AuPmmngrFree((void*)physical_address);

	mov	rcx, QWORD PTR physical_address$2[rsp]
	call	AuPmmngrFree
$LN1@FreeImage:

; 71   : 	}

	jmp	SHORT $LN3@FreeImage
$LN2@FreeImage:

; 72   : 
; 73   : 	AuVMArea *image_area = AuVMAreaGet(proc, proc->_image_base_);

	mov	rax, QWORD PTR proc$[rsp]
	mov	rdx, QWORD PTR [rax+40]
	mov	rcx, QWORD PTR proc$[rsp]
	call	?AuVMAreaGet@@YAPEAU_vm_area_@@PEAU_au_proc_@@_K@Z ; AuVMAreaGet
	mov	QWORD PTR image_area$[rsp], rax

; 74   : 	AuRemoveVMArea(proc, image_area);

	mov	rdx, QWORD PTR image_area$[rsp]
	mov	rcx, QWORD PTR proc$[rsp]
	call	?AuRemoveVMArea@@YAXPEAU_au_proc_@@PEAU_vm_area_@@@Z ; AuRemoveVMArea

; 75   : }

	add	rsp, 88					; 00000058H
	ret	0
?FreeImage@@YAXPEAU_au_proc_@@@Z ENDP			; FreeImage
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\clean.cpp
_TEXT	SEGMENT
i$1 = 32
physaddr$2 = 40
addr$3 = 48
location$ = 56
cr3$ = 80
ptr$ = 88
?FreeUserStack@@YAXPEA_KPEAX@Z PROC			; FreeUserStack

; 48   : void FreeUserStack(uint64_t* cr3, void* ptr) {

$LN7:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 72					; 00000048H

; 49   : 	uint64_t location = (uint64_t)ptr;

	mov	rax, QWORD PTR ptr$[rsp]
	mov	QWORD PTR location$[rsp], rax

; 50   : 
; 51   : 	for (int i = 0; i < PROCESS_USER_STACK_SZ / 4096; i++) {

	mov	DWORD PTR i$1[rsp], 0
	jmp	SHORT $LN4@FreeUserSt
$LN3@FreeUserSt:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN4@FreeUserSt:
	cmp	DWORD PTR i$1[rsp], 128			; 00000080H
	jge	SHORT $LN2@FreeUserSt

; 52   : 		void* addr = AuGetPhysicalAddressEx(cr3, + i * 4096);

	imul	eax, DWORD PTR i$1[rsp], 4096		; 00001000H
	cdqe
	mov	rdx, rax
	mov	rcx, QWORD PTR cr3$[rsp]
	call	AuGetPhysicalAddressEx
	mov	QWORD PTR addr$3[rsp], rax

; 53   : 		uint64_t physaddr = (uint64_t)V2P((uint64_t)addr);

	mov	rcx, QWORD PTR addr$3[rsp]
	call	V2P
	mov	QWORD PTR physaddr$2[rsp], rax

; 54   : 		if (physaddr != 0)

	cmp	QWORD PTR physaddr$2[rsp], 0
	je	SHORT $LN1@FreeUserSt

; 55   : 			AuPmmngrFree((void*)physaddr);

	mov	rcx, QWORD PTR physaddr$2[rsp]
	call	AuPmmngrFree
$LN1@FreeUserSt:

; 56   : 	}

	jmp	SHORT $LN3@FreeUserSt
$LN2@FreeUserSt:

; 57   : }

	add	rsp, 72					; 00000048H
	ret	0
?FreeUserStack@@YAXPEA_KPEAX@Z ENDP			; FreeUserStack
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\clean.cpp
_TEXT	SEGMENT
i$1 = 32
i$2 = 36
t_$3 = 40
entry$4 = 48
stack_$5 = 56
uentry$ = 64
area$6 = 72
phys$7 = 80
stack$8 = 88
id$ = 96
stack_$ = 104
tv174 = 112
parent$ = 144
killable$ = 152
?AuProcessClean@@YAXPEAU_au_proc_@@0@Z PROC		; AuProcessClean

; 121  : void AuProcessClean(AuProcess* parent, AuProcess* killable) {

$LN14:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 136				; 00000088H

; 122  : 	int id = killable->proc_id;

	mov	rax, QWORD PTR killable$[rsp]
	mov	eax, DWORD PTR [rax]
	mov	DWORD PTR id$[rsp], eax

; 123  : 	
; 124  : 	uint64_t stack_ = killable->_main_stack_ - PROCESS_USER_STACK_SZ;

	mov	rax, QWORD PTR killable$[rsp]
	mov	rax, QWORD PTR [rax+48]
	sub	rax, 524288				; 00080000H
	mov	QWORD PTR stack_$[rsp], rax

; 125  : 	FreeUserStack(killable->cr3,(void*)stack_);

	mov	rdx, QWORD PTR stack_$[rsp]
	mov	rax, QWORD PTR killable$[rsp]
	mov	rcx, QWORD PTR [rax+24]
	call	?FreeUserStack@@YAXPEA_KPEAX@Z		; FreeUserStack

; 126  : 	/* free up shm mappings */
; 127  : 
; 128  : 	/* free up image base + image size*/
; 129  : 	FreeImage(killable);

	mov	rcx, QWORD PTR killable$[rsp]
	call	?FreeImage@@YAXPEAU_au_proc_@@@Z	; FreeImage

; 130  : 
; 131  : 	/* free up vmareas */
; 132  : 	for (int i = 0; i < killable->vmareas->pointer; i++) {

	mov	DWORD PTR i$2[rsp], 0
	jmp	SHORT $LN11@AuProcessC
$LN10@AuProcessC:
	mov	eax, DWORD PTR i$2[rsp]
	inc	eax
	mov	DWORD PTR i$2[rsp], eax
$LN11@AuProcessC:
	mov	rax, QWORD PTR killable$[rsp]
	mov	rax, QWORD PTR [rax+1072]
	mov	eax, DWORD PTR [rax]
	cmp	DWORD PTR i$2[rsp], eax
	jae	SHORT $LN9@AuProcessC

; 133  : 		AuVMArea* area = (AuVMArea*)list_remove(killable->vmareas, i);

	mov	edx, DWORD PTR i$2[rsp]
	mov	rax, QWORD PTR killable$[rsp]
	mov	rcx, QWORD PTR [rax+1072]
	call	list_remove
	mov	QWORD PTR area$6[rsp], rax

; 134  : 		if (area)

	cmp	QWORD PTR area$6[rsp], 0
	je	SHORT $LN8@AuProcessC

; 135  : 			kfree(area);

	mov	rcx, QWORD PTR area$6[rsp]
	call	kfree
$LN8@AuProcessC:

; 136  : 	}

	jmp	SHORT $LN10@AuProcessC
$LN9@AuProcessC:

; 137  : 	kfree(killable->vmareas);

	mov	rax, QWORD PTR killable$[rsp]
	mov	rcx, QWORD PTR [rax+1072]
	call	kfree

; 138  : 
; 139  : 
; 140  : 	/* finally free up all threads */
; 141  : 	for (int i = 1; i < MAX_THREADS_PER_PROCESS -1 ; i++) {

	mov	DWORD PTR i$1[rsp], 1
	jmp	SHORT $LN7@AuProcessC
$LN6@AuProcessC:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN7@AuProcessC:
	cmp	DWORD PTR i$1[rsp], 59			; 0000003bH
	jge	$LN5@AuProcessC

; 142  : 		AuThread *t_ = killable->threads[i];

	movsxd	rax, DWORD PTR i$1[rsp]
	mov	rcx, QWORD PTR killable$[rsp]
	mov	rax, QWORD PTR [rcx+rax*8+96]
	mov	QWORD PTR t_$3[rsp], rax

; 143  : 		if (t_) {

	cmp	QWORD PTR t_$3[rsp], 0
	je	$LN4@AuProcessC

; 144  : 			SeTextOut("cleaning thread -> %x %s\n", t_, t_->name);

	mov	rax, QWORD PTR t_$3[rsp]
	add	rax, 284				; 0000011cH
	mov	r8, rax
	mov	rdx, QWORD PTR t_$3[rsp]
	lea	rcx, OFFSET FLAT:$SG4080
	call	SeTextOut

; 145  : 			AuThreadCleanTrash(t_);

	mov	rcx, QWORD PTR t_$3[rsp]
	call	?AuThreadCleanTrash@@YAXPEAU_au_thread_@@@Z ; AuThreadCleanTrash

; 146  : 			AuUserEntry* entry = t_->uentry;

	mov	rax, QWORD PTR t_$3[rsp]
	mov	rax, QWORD PTR [rax+641]
	mov	QWORD PTR entry$4[rsp], rax

; 147  : 			if (entry) {

	cmp	QWORD PTR entry$4[rsp], 0
	je	SHORT $LN3@AuProcessC

; 148  : 				uint64_t stack = entry->stackBase + 32;

	mov	rax, QWORD PTR entry$4[rsp]
	mov	rax, QWORD PTR [rax+52]
	add	rax, 32					; 00000020H
	mov	QWORD PTR stack$8[rsp], rax

; 149  : 				uint64_t stack_ = stack - PROCESS_USER_STACK_SZ;

	mov	rax, QWORD PTR stack$8[rsp]
	sub	rax, 524288				; 00080000H
	mov	QWORD PTR stack_$5[rsp], rax

; 150  : 				SeTextOut("Stack_ -> %x stack -> %x\r\n", stack_, stack);

	mov	r8, QWORD PTR stack$8[rsp]
	mov	rdx, QWORD PTR stack_$5[rsp]
	lea	rcx, OFFSET FLAT:$SG4085
	call	SeTextOut

; 151  : 				FreeUserStack(killable->cr3, (void*)stack_);

	mov	rdx, QWORD PTR stack_$5[rsp]
	mov	rax, QWORD PTR killable$[rsp]
	mov	rcx, QWORD PTR [rax+24]
	call	?FreeUserStack@@YAXPEA_KPEAX@Z		; FreeUserStack

; 152  : 				killable->_user_stack_index_ -= PROCESS_USER_STACK_SZ;

	mov	rax, QWORD PTR killable$[rsp]
	mov	rax, QWORD PTR [rax+56]
	sub	rax, 524288				; 00080000H
	mov	rcx, QWORD PTR killable$[rsp]
	mov	QWORD PTR [rcx+56], rax
$LN3@AuProcessC:

; 153  : 			}
; 154  : 			SeTextOut("Thread user stack freed \r\n");

	lea	rcx, OFFSET FLAT:$SG4087
	call	SeTextOut

; 155  : 			AuThreadFree(killable, t_);

	mov	rdx, QWORD PTR t_$3[rsp]
	mov	rcx, QWORD PTR killable$[rsp]
	call	?AuThreadFree@@YAXPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuThreadFree

; 156  : 			SeTextOut("Thread freed \r\n");

	lea	rcx, OFFSET FLAT:$SG4088
	call	SeTextOut
$LN4@AuProcessC:

; 157  : 		}
; 158  : 	}

	jmp	$LN6@AuProcessC
$LN5@AuProcessC:

; 159  : 
; 160  : 	AuUserEntry* uentry = killable->main_thread->uentry;

	mov	rax, QWORD PTR killable$[rsp]
	mov	rax, QWORD PTR [rax+72]
	mov	rax, QWORD PTR [rax+641]
	mov	QWORD PTR uentry$[rsp], rax

; 161  : 	SeTextOut("Uenty -> %x \r\n", uentry);

	mov	rdx, QWORD PTR uentry$[rsp]
	lea	rcx, OFFSET FLAT:$SG4090
	call	SeTextOut

; 162  : 	if (uentry->argvaddr != 0) {

	mov	rax, QWORD PTR uentry$[rsp]
	cmp	QWORD PTR [rax+36], 0
	je	SHORT $LN2@AuProcessC

; 163  : 		void* phys = AuGetPhysicalAddressEx(killable->cr3, 0x4000);

	mov	edx, 16384				; 00004000H
	mov	rax, QWORD PTR killable$[rsp]
	mov	rcx, QWORD PTR [rax+24]
	call	AuGetPhysicalAddressEx
	mov	QWORD PTR phys$7[rsp], rax

; 164  : 		if (phys) {

	cmp	QWORD PTR phys$7[rsp], 0
	je	SHORT $LN1@AuProcessC

; 165  : 			AuPmmngrFree((void*)P2V((size_t)phys));

	mov	rcx, QWORD PTR phys$7[rsp]
	call	P2V
	mov	rcx, rax
	call	AuPmmngrFree
$LN1@AuProcessC:
$LN2@AuProcessC:

; 166  : 		}
; 167  : 	}
; 168  : 
; 169  : 	/* clean the main thread externally*/
; 170  : 	AuThreadCleanTrash(killable->main_thread);

	mov	rax, QWORD PTR killable$[rsp]
	mov	rcx, QWORD PTR [rax+72]
	call	?AuThreadCleanTrash@@YAXPEAU_au_thread_@@@Z ; AuThreadCleanTrash

; 171  : 	AuThreadFree(killable, killable->main_thread);

	mov	rax, QWORD PTR killable$[rsp]
	mov	rdx, QWORD PTR [rax+72]
	mov	rcx, QWORD PTR killable$[rsp]
	call	?AuThreadFree@@YAXPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuThreadFree

; 172  : 
; 173  : 	/* release the process slot */
; 174  : 
; 175  : 	AuPmmngrFree((void*)V2P((size_t)killable->cr3));

	mov	rax, QWORD PTR killable$[rsp]
	mov	rcx, QWORD PTR [rax+24]
	call	V2P
	mov	rcx, rax
	call	AuPmmngrFree

; 176  : 	AuRemoveProcess(0, killable);

	mov	rdx, QWORD PTR killable$[rsp]
	xor	ecx, ecx
	call	?AuRemoveProcess@@YAXPEAU_au_proc_@@0@Z	; AuRemoveProcess

; 177  : 	SeTextOut("Used RAM -> %d GB \ Avail -> %d GB \r\n", ((AuPmmngrGetFreeMem() * PAGE_SIZE) / 1024 / 1024 / 1024),
; 178  : 		(AuPmmngrGetTotalMem() * PAGE_SIZE / 1024 / 1024 / 1024));

	call	?AuPmmngrGetTotalMem@@YA_KXZ		; AuPmmngrGetTotalMem
	imul	rax, rax, 4096				; 00001000H
	xor	edx, edx
	mov	ecx, 1024				; 00000400H
	div	rcx
	xor	edx, edx
	mov	ecx, 1024				; 00000400H
	div	rcx
	xor	edx, edx
	mov	ecx, 1024				; 00000400H
	div	rcx
	mov	QWORD PTR tv174[rsp], rax
	call	?AuPmmngrGetFreeMem@@YA_KXZ		; AuPmmngrGetFreeMem
	imul	rax, rax, 4096				; 00001000H
	xor	edx, edx
	mov	ecx, 1024				; 00000400H
	div	rcx
	xor	edx, edx
	mov	ecx, 1024				; 00000400H
	div	rcx
	xor	edx, edx
	mov	ecx, 1024				; 00000400H
	div	rcx
	mov	rcx, QWORD PTR tv174[rsp]
	mov	r8, rcx
	mov	rdx, rax
	lea	rcx, OFFSET FLAT:$SG4098
	call	SeTextOut

; 179  : 	SeTextOut("Process cleaned \r\n");

	lea	rcx, OFFSET FLAT:$SG4099
	call	SeTextOut

; 180  : }

	add	rsp, 136				; 00000088H
	ret	0
?AuProcessClean@@YAXPEAU_au_proc_@@0@Z ENDP		; AuProcessClean
_TEXT	ENDS
END
