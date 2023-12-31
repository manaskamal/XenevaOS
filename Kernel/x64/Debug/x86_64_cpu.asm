; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?_tss@@3PEAU0@EA				; _tss
PUBLIC	?_fxsave@@3_NA					; _fxsave
PUBLIC	?__ApStarted@@3_NA				; __ApStarted
_BSS	SEGMENT
?_tss@@3PEAU0@EA DQ 01H DUP (?)				; _tss
?_fxsave@@3_NA DB 01H DUP (?)				; _fxsave
	ALIGN	4

?__ApStarted@@3_NA DB 01H DUP (?)			; __ApStarted
_BSS	ENDS
PUBLIC	?x86_64_enable_syscall_ext@@YAXXZ		; x86_64_enable_syscall_ext
PUBLIC	?x86_64_init_user@@YAX_K@Z			; x86_64_init_user
PUBLIC	?x86_64_init_user_ap@@YAX_K@Z			; x86_64_init_user_ap
PUBLIC	?x86_64_get_tss@@YAPEAU_tss@@XZ			; x86_64_get_tss
PUBLIC	?x86_64_hal_cpu_feature_enable@@YAXXZ		; x86_64_hal_cpu_feature_enable
PUBLIC	?x86_64_is_cpu_fxsave_supported@@YA_NXZ		; x86_64_is_cpu_fxsave_supported
PUBLIC	?x86_64_cpu_msi_address@@YA_KPEA_K_KIEE@Z	; x86_64_cpu_msi_address
PUBLIC	?x86_64_cpu_initialize@@YAXE@Z			; x86_64_cpu_initialize
PUBLIC	?x86_64_set_ap_start_bit@@YAX_N@Z		; x86_64_set_ap_start_bit
PUBLIC	?x86_64_initialise_syscall@@YAXXZ		; x86_64_initialise_syscall
PUBLIC	?cpu_read_tsc@@YA_KXZ				; cpu_read_tsc
PUBLIC	?_ICRDest@@YA_KI@Z				; _ICRDest
PUBLIC	?_ICRBusy@@YA_NXZ				; _ICRBusy
EXTRN	?AuPerCPUSetKernelTSS@@YAXPEAU_tss@@@Z:PROC	; AuPerCPUSetKernelTSS
EXTRN	x64_read_msr:PROC
EXTRN	x64_write_msr:PROC
EXTRN	x64_pause:PROC
EXTRN	x64_cpuid:PROC
EXTRN	x64_read_cr0:PROC
EXTRN	x64_read_cr4:PROC
EXTRN	x64_write_cr0:PROC
EXTRN	x64_write_cr4:PROC
EXTRN	x64_sgdt:PROC
EXTRN	x64_rdtsc:PROC
EXTRN	?ReadAPICRegister@@YA_KG@Z:PROC			; ReadAPICRegister
EXTRN	?WriteAPICRegister@@YAXG_K@Z:PROC		; WriteAPICRegister
EXTRN	?X2APICSupported@@YA_NXZ:PROC			; X2APICSupported
EXTRN	?x86_64_ap_init@@YAXPEAX@Z:PROC			; x86_64_ap_init
EXTRN	AuPmmngrAlloc:PROC
EXTRN	P2V:PROC
EXTRN	AuGetRootPageTable:PROC
EXTRN	syscall_entry:PROC
EXTRN	x64_syscall_entry_compat:PROC
pdata	SEGMENT
$pdata$?x86_64_enable_syscall_ext@@YAXXZ DD imagerel $LN3
	DD	imagerel $LN3+68
	DD	imagerel $unwind$?x86_64_enable_syscall_ext@@YAXXZ
$pdata$?x86_64_init_user@@YAX_K@Z DD imagerel $LN8
	DD	imagerel $LN8+190
	DD	imagerel $unwind$?x86_64_init_user@@YAX_K@Z
$pdata$?x86_64_init_user_ap@@YAX_K@Z DD imagerel $LN8
	DD	imagerel $LN8+198
	DD	imagerel $unwind$?x86_64_init_user_ap@@YAX_K@Z
$pdata$?x86_64_hal_cpu_feature_enable@@YAXXZ DD imagerel $LN10
	DD	imagerel $LN10+270
	DD	imagerel $unwind$?x86_64_hal_cpu_feature_enable@@YAXXZ
$pdata$?x86_64_cpu_msi_address@@YA_KPEA_K_KIEE@Z DD imagerel $LN7
	DD	imagerel $LN7+131
	DD	imagerel $unwind$?x86_64_cpu_msi_address@@YA_KPEA_K_KIEE@Z
$pdata$?x86_64_cpu_initialize@@YAXE@Z DD imagerel $LN23
	DD	imagerel $LN23+496
	DD	imagerel $unwind$?x86_64_cpu_initialize@@YAXE@Z
$pdata$?x86_64_initialise_syscall@@YAXXZ DD imagerel $LN3
	DD	imagerel $LN3+110
	DD	imagerel $unwind$?x86_64_initialise_syscall@@YAXXZ
$pdata$?cpu_read_tsc@@YA_KXZ DD imagerel $LN3
	DD	imagerel $LN3+65
	DD	imagerel $unwind$?cpu_read_tsc@@YA_KXZ
$pdata$?_ICRDest@@YA_KI@Z DD imagerel $LN5
	DD	imagerel $LN5+45
	DD	imagerel $unwind$?_ICRDest@@YA_KI@Z
$pdata$?_ICRBusy@@YA_NXZ DD imagerel $LN5
	DD	imagerel $LN5+52
	DD	imagerel $unwind$?_ICRBusy@@YA_NXZ
pdata	ENDS
xdata	SEGMENT
$unwind$?x86_64_enable_syscall_ext@@YAXXZ DD 010401H
	DD	06204H
$unwind$?x86_64_init_user@@YAX_K@Z DD 010901H
	DD	0a209H
$unwind$?x86_64_init_user_ap@@YAX_K@Z DD 010901H
	DD	0a209H
$unwind$?x86_64_hal_cpu_feature_enable@@YAXXZ DD 010401H
	DD	0e204H
$unwind$?x86_64_cpu_msi_address@@YA_KPEA_K_KIEE@Z DD 011801H
	DD	02218H
$unwind$?x86_64_cpu_initialize@@YAXE@Z DD 010801H
	DD	0e208H
$unwind$?x86_64_initialise_syscall@@YAXXZ DD 010401H
	DD	06204H
$unwind$?cpu_read_tsc@@YA_KXZ DD 010401H
	DD	06204H
$unwind$?_ICRDest@@YA_KI@Z DD 010801H
	DD	04208H
$unwind$?_ICRBusy@@YA_NXZ DD 010401H
	DD	06204H
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
tv68 = 32
?_ICRBusy@@YA_NXZ PROC					; _ICRBusy

; 185  : bool _ICRBusy() {

$LN5:
	sub	rsp, 56					; 00000038H

; 186  : 	return (ReadAPICRegister(LAPIC_REGISTER_ICR) & (1 << 12)) != 0;

	mov	cx, 48					; 00000030H
	call	?ReadAPICRegister@@YA_KG@Z		; ReadAPICRegister
	and	rax, 4096				; 00001000H
	test	rax, rax
	je	SHORT $LN3@ICRBusy
	mov	DWORD PTR tv68[rsp], 1
	jmp	SHORT $LN4@ICRBusy
$LN3@ICRBusy:
	mov	DWORD PTR tv68[rsp], 0
$LN4@ICRBusy:
	movzx	eax, BYTE PTR tv68[rsp]

; 187  : }

	add	rsp, 56					; 00000038H
	ret	0
?_ICRBusy@@YA_NXZ ENDP					; _ICRBusy
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
processor$ = 48
?_ICRDest@@YA_KI@Z PROC					; _ICRDest

; 178  : uint64_t _ICRDest(uint32_t processor) {

$LN5:
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 40					; 00000028H

; 179  : 	if (X2APICSupported())

	call	?X2APICSupported@@YA_NXZ		; X2APICSupported
	movzx	eax, al
	test	eax, eax
	je	SHORT $LN2@ICRDest

; 180  : 		return ((uint64_t)processor << 32);

	mov	eax, DWORD PTR processor$[rsp]
	shl	rax, 32					; 00000020H
	jmp	SHORT $LN3@ICRDest

; 181  : 	else

	jmp	SHORT $LN1@ICRDest
$LN2@ICRDest:

; 182  : 		return ((uint64_t)processor << 56);

	mov	eax, DWORD PTR processor$[rsp]
	shl	rax, 56					; 00000038H
$LN1@ICRDest:
$LN3@ICRDest:

; 183  : }

	add	rsp, 40					; 00000028H
	ret	0
?_ICRDest@@YA_KI@Z ENDP					; _ICRDest
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
hi$ = 32
lo$ = 36
count$ = 40
?cpu_read_tsc@@YA_KXZ PROC				; cpu_read_tsc

; 275  : uint64_t cpu_read_tsc(){

$LN3:
	sub	rsp, 56					; 00000038H

; 276  : 	uint32_t hi = 0;

	mov	DWORD PTR hi$[rsp], 0

; 277  : 	uint32_t lo = 0;

	mov	DWORD PTR lo$[rsp], 0

; 278  : 	x64_rdtsc(&hi, &lo);

	lea	rdx, QWORD PTR lo$[rsp]
	lea	rcx, QWORD PTR hi$[rsp]
	call	x64_rdtsc

; 279  : 	uint64_t count = (((uint64_t)hi << 32UL) | (uint64_t)lo);

	mov	eax, DWORD PTR hi$[rsp]
	shl	rax, 32					; 00000020H
	mov	ecx, DWORD PTR lo$[rsp]
	or	rax, rcx
	mov	QWORD PTR count$[rsp], rax

; 280  : 	return count;

	mov	rax, QWORD PTR count$[rsp]

; 281  : }

	add	rsp, 56					; 00000038H
	ret	0
?cpu_read_tsc@@YA_KXZ ENDP				; cpu_read_tsc
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
sysret_sel$ = 32
syscall_sel$ = 40
?x86_64_initialise_syscall@@YAXXZ PROC			; x86_64_initialise_syscall

; 262  : void x86_64_initialise_syscall() {

$LN3:
	sub	rsp, 56					; 00000038H

; 263  : 	uint64_t syscall_sel = SEGVAL(GDT_ENTRY_KERNEL_CODE, 0);

	mov	QWORD PTR syscall_sel$[rsp], 8

; 264  : 	uint64_t sysret_sel = SEGVAL(GDT_ENTRY_USER_CODE32, 3);

	mov	QWORD PTR sysret_sel$[rsp], 27

; 265  : 
; 266  : 	x64_write_msr(IA32_STAR, (sysret_sel << 48) | (syscall_sel << 32));

	mov	rax, QWORD PTR sysret_sel$[rsp]
	shl	rax, 48					; 00000030H
	mov	rcx, QWORD PTR syscall_sel$[rsp]
	shl	rcx, 32					; 00000020H
	or	rax, rcx
	mov	rdx, rax
	mov	ecx, -1073741695			; c0000081H
	call	x64_write_msr

; 267  : 	x64_write_msr(IA32_LSTAR, (size_t)&syscall_entry);

	lea	rdx, OFFSET FLAT:syscall_entry
	mov	ecx, -1073741694			; c0000082H
	call	x64_write_msr

; 268  : 	x64_write_msr(IA32_SFMASK, IA32_EFLAGS_INTR | IA32_EFLAGS_DIRF);

	mov	edx, 1536				; 00000600H
	mov	ecx, -1073741692			; c0000084H
	call	x64_write_msr

; 269  : 	x64_write_msr(IA32_CSTAR, (size_t)&x64_syscall_entry_compat);

	lea	rdx, OFFSET FLAT:x64_syscall_entry_compat
	mov	ecx, -1073741693			; c0000083H
	call	x64_write_msr

; 270  : }

	add	rsp, 56					; 00000038H
	ret	0
?x86_64_initialise_syscall@@YAXXZ ENDP			; x86_64_initialise_syscall
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
value$ = 8
?x86_64_set_ap_start_bit@@YAX_N@Z PROC			; x86_64_set_ap_start_bit

; 252  : void x86_64_set_ap_start_bit(bool value) {

	mov	BYTE PTR [rsp+8], cl

; 253  : 	__ApStarted = value;

	movzx	eax, BYTE PTR value$[rsp]
	mov	BYTE PTR ?__ApStarted@@3_NA, al		; __ApStarted

; 254  : }

	ret	0
?x86_64_set_ap_start_bit@@YAX_N@Z ENDP			; x86_64_set_ap_start_bit
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
i$1 = 32
i$2 = 36
i$3 = 40
ap_aligned_address$ = 48
cpu$4 = 56
cpu_struc$5 = 64
apdata$ = 72
startup_ipi$6 = 80
pml4$ = 88
stack_address$7 = 96
ap_init_address$ = 104
num_cpu$ = 128
?x86_64_cpu_initialize@@YAXE@Z PROC			; x86_64_cpu_initialize

; 195  : void x86_64_cpu_initialize(uint8_t num_cpu) {

$LN23:
	mov	BYTE PTR [rsp+8], cl
	sub	rsp, 120				; 00000078H

; 196  : 	if (num_cpu == 0)

	movzx	eax, BYTE PTR num_cpu$[rsp]
	test	eax, eax
	jne	SHORT $LN20@x86_64_cpu

; 197  : 		return;

	jmp	$LN21@x86_64_cpu
$LN20@x86_64_cpu:

; 198  : 
; 199  : 	//! fixed address
; 200  : 	uint64_t *apdata = (uint64_t*)0xA000;

	mov	QWORD PTR apdata$[rsp], 40960		; 0000a000H

; 201  : 	uint64_t ap_init_address = (uint64_t)x86_64_ap_init;

	lea	rax, OFFSET FLAT:?x86_64_ap_init@@YAXPEAX@Z ; x86_64_ap_init
	mov	QWORD PTR ap_init_address$[rsp], rax

; 202  : 	uint64_t ap_aligned_address = (uint64_t)apdata;

	mov	rax, QWORD PTR apdata$[rsp]
	mov	QWORD PTR ap_aligned_address$[rsp], rax

; 203  : 
; 204  : 	uint64_t *pml4 = (uint64_t*)AuGetRootPageTable();

	call	AuGetRootPageTable
	mov	QWORD PTR pml4$[rsp], rax

; 205  : 
; 206  : 	for (int i = 1; i <= num_cpu; i++) {

	mov	DWORD PTR i$1[rsp], 1
	jmp	SHORT $LN19@x86_64_cpu
$LN18@x86_64_cpu:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN19@x86_64_cpu:
	movzx	eax, BYTE PTR num_cpu$[rsp]
	cmp	DWORD PTR i$1[rsp], eax
	jg	$LN17@x86_64_cpu

; 207  : 
; 208  : 		/* In SMP Mode : no more than 8 cpus */
; 209  : 		if (i == 8)

	cmp	DWORD PTR i$1[rsp], 8
	jne	SHORT $LN16@x86_64_cpu

; 210  : 			break;

	jmp	$LN17@x86_64_cpu
$LN16@x86_64_cpu:

; 211  : 	
; 212  : 		__ApStarted = false;

	mov	BYTE PTR ?__ApStarted@@3_NA, 0		; __ApStarted

; 213  : 
; 214  : 
; 215  : 		void *stack_address = AuPmmngrAlloc();

	call	AuPmmngrAlloc
	mov	QWORD PTR stack_address$7[rsp], rax

; 216  : 		*(uint64_t*)(ap_aligned_address + 8) = (uint64_t)pml4;

	mov	rax, QWORD PTR ap_aligned_address$[rsp]
	mov	rcx, QWORD PTR pml4$[rsp]
	mov	QWORD PTR [rax+8], rcx

; 217  : 		*(uint64_t*)(ap_aligned_address + 16) = (uint64_t)stack_address;

	mov	rax, QWORD PTR ap_aligned_address$[rsp]
	mov	rcx, QWORD PTR stack_address$7[rsp]
	mov	QWORD PTR [rax+16], rcx

; 218  : 		*(uint64_t*)(ap_aligned_address + 24) = ap_init_address;

	mov	rax, QWORD PTR ap_aligned_address$[rsp]
	mov	rcx, QWORD PTR ap_init_address$[rsp]
	mov	QWORD PTR [rax+24], rcx

; 219  : 		*(uint64_t*)(ap_aligned_address + 32) = (uint64_t)P2V((size_t)AuPmmngrAlloc());

	call	AuPmmngrAlloc
	mov	rcx, rax
	call	P2V
	mov	rcx, QWORD PTR ap_aligned_address$[rsp]
	mov	QWORD PTR [rcx+32], rax

; 220  : 		void* cpu_struc = (void*)P2V((uint64_t)AuPmmngrAlloc());

	call	AuPmmngrAlloc
	mov	rcx, rax
	call	P2V
	mov	QWORD PTR cpu_struc$5[rsp], rax

; 221  : 		CPUStruc *cpu = (CPUStruc*)cpu_struc;

	mov	rax, QWORD PTR cpu_struc$5[rsp]
	mov	QWORD PTR cpu$4[rsp], rax

; 222  : 		cpu->cpu_id = i;

	mov	rax, QWORD PTR cpu$4[rsp]
	movzx	ecx, BYTE PTR i$1[rsp]
	mov	BYTE PTR [rax], cl

; 223  : 		cpu->au_current_thread = 0;

	mov	rax, QWORD PTR cpu$4[rsp]
	mov	QWORD PTR [rax+1], 0

; 224  : 		cpu->kernel_tss = 0;

	mov	rax, QWORD PTR cpu$4[rsp]
	mov	QWORD PTR [rax+9], 0

; 225  : 		*(uint64_t*)(ap_aligned_address + 40) = (uint64_t)cpu_struc;

	mov	rax, QWORD PTR ap_aligned_address$[rsp]
	mov	rcx, QWORD PTR cpu_struc$5[rsp]
	mov	QWORD PTR [rax+40], rcx

; 226  : 
; 227  : 
; 228  : 		WriteAPICRegister(LAPIC_REGISTER_ICR, _ICRDest(i) | 0x4500);

	mov	ecx, DWORD PTR i$1[rsp]
	call	?_ICRDest@@YA_KI@Z			; _ICRDest
	or	rax, 17664				; 00004500H
	mov	rdx, rax
	mov	cx, 48					; 00000030H
	call	?WriteAPICRegister@@YAXG_K@Z		; WriteAPICRegister
$LN15@x86_64_cpu:

; 229  : 		while (_ICRBusy());

	call	?_ICRBusy@@YA_NXZ			; _ICRBusy
	movzx	eax, al
	test	eax, eax
	je	SHORT $LN14@x86_64_cpu
	jmp	SHORT $LN15@x86_64_cpu
$LN14@x86_64_cpu:

; 230  : 
; 231  : 
; 232  : 		size_t startup_ipi = _ICRDest(i) | 0x4600 | ((size_t)apdata >> 12);

	mov	ecx, DWORD PTR i$1[rsp]
	call	?_ICRDest@@YA_KI@Z			; _ICRDest
	or	rax, 17920				; 00004600H
	mov	rcx, QWORD PTR apdata$[rsp]
	shr	rcx, 12
	or	rax, rcx
	mov	QWORD PTR startup_ipi$6[rsp], rax

; 233  : 		WriteAPICRegister(LAPIC_REGISTER_ICR, startup_ipi);

	mov	rdx, QWORD PTR startup_ipi$6[rsp]
	mov	cx, 48					; 00000030H
	call	?WriteAPICRegister@@YAXG_K@Z		; WriteAPICRegister
$LN13@x86_64_cpu:

; 234  : 		while (_ICRBusy());

	call	?_ICRBusy@@YA_NXZ			; _ICRBusy
	movzx	eax, al
	test	eax, eax
	je	SHORT $LN12@x86_64_cpu
	jmp	SHORT $LN13@x86_64_cpu
$LN12@x86_64_cpu:

; 235  : 		for (int i = 0; i < 10000000; i++)

	mov	DWORD PTR i$3[rsp], 0
	jmp	SHORT $LN11@x86_64_cpu
$LN10@x86_64_cpu:
	mov	eax, DWORD PTR i$3[rsp]
	inc	eax
	mov	DWORD PTR i$3[rsp], eax
$LN11@x86_64_cpu:
	cmp	DWORD PTR i$3[rsp], 10000000		; 00989680H
	jge	SHORT $LN9@x86_64_cpu

; 236  : 			;

	jmp	SHORT $LN10@x86_64_cpu
$LN9@x86_64_cpu:

; 237  : 		WriteAPICRegister(LAPIC_REGISTER_ICR, startup_ipi);

	mov	rdx, QWORD PTR startup_ipi$6[rsp]
	mov	cx, 48					; 00000030H
	call	?WriteAPICRegister@@YAXG_K@Z		; WriteAPICRegister
$LN8@x86_64_cpu:

; 238  : 		while (_ICRBusy());

	call	?_ICRBusy@@YA_NXZ			; _ICRBusy
	movzx	eax, al
	test	eax, eax
	je	SHORT $LN7@x86_64_cpu
	jmp	SHORT $LN8@x86_64_cpu
$LN7@x86_64_cpu:

; 239  : 
; 240  : 		for (int i = 0; i < 10000000; i++)

	mov	DWORD PTR i$2[rsp], 0
	jmp	SHORT $LN6@x86_64_cpu
$LN5@x86_64_cpu:
	mov	eax, DWORD PTR i$2[rsp]
	inc	eax
	mov	DWORD PTR i$2[rsp], eax
$LN6@x86_64_cpu:
	cmp	DWORD PTR i$2[rsp], 10000000		; 00989680H
	jge	SHORT $LN4@x86_64_cpu

; 241  : 			;

	jmp	SHORT $LN5@x86_64_cpu
$LN4@x86_64_cpu:
$LN3@x86_64_cpu:

; 242  : 		do {
; 243  : 			x64_pause();

	call	x64_pause

; 244  : 		} while (!__ApStarted);

	movzx	eax, BYTE PTR ?__ApStarted@@3_NA	; __ApStarted
	test	eax, eax
	je	SHORT $LN3@x86_64_cpu

; 245  : 	}

	jmp	$LN18@x86_64_cpu
$LN17@x86_64_cpu:
$LN21@x86_64_cpu:

; 246  : }

	add	rsp, 120				; 00000078H
	ret	0
?x86_64_cpu_initialize@@YAXE@Z ENDP			; x86_64_cpu_initialize
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
tv67 = 0
tv72 = 4
data$ = 32
vector$ = 40
processor$ = 48
edge$ = 56
deassert$ = 64
?x86_64_cpu_msi_address@@YA_KPEA_K_KIEE@Z PROC		; x86_64_cpu_msi_address

; 169  : uint64_t x86_64_cpu_msi_address(uint64_t* data, size_t vector, uint32_t processor, uint8_t edge, uint8_t deassert) {

$LN7:
	mov	BYTE PTR [rsp+32], r9b
	mov	DWORD PTR [rsp+24], r8d
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 24

; 170  : 	*data = (vector & 0xFF) | (edge == 1 ? 0 : (1 << 15)) | (deassert == 1 ? 0 : (1 << 14));

	movzx	eax, BYTE PTR edge$[rsp]
	cmp	eax, 1
	jne	SHORT $LN3@x86_64_cpu
	mov	DWORD PTR tv67[rsp], 0
	jmp	SHORT $LN4@x86_64_cpu
$LN3@x86_64_cpu:
	mov	DWORD PTR tv67[rsp], 32768		; 00008000H
$LN4@x86_64_cpu:
	movzx	eax, BYTE PTR deassert$[rsp]
	cmp	eax, 1
	jne	SHORT $LN5@x86_64_cpu
	mov	DWORD PTR tv72[rsp], 0
	jmp	SHORT $LN6@x86_64_cpu
$LN5@x86_64_cpu:
	mov	DWORD PTR tv72[rsp], 16384		; 00004000H
$LN6@x86_64_cpu:
	mov	rax, QWORD PTR vector$[rsp]
	and	rax, 255				; 000000ffH
	movsxd	rcx, DWORD PTR tv67[rsp]
	or	rax, rcx
	movsxd	rcx, DWORD PTR tv72[rsp]
	or	rax, rcx
	mov	rcx, QWORD PTR data$[rsp]
	mov	QWORD PTR [rcx], rax

; 171  : 	//*data = low;
; 172  : 	return (0xFEE00000 | (processor << 12));

	mov	eax, DWORD PTR processor$[rsp]
	shl	eax, 12
	or	eax, -18874368				; fee00000H
	mov	eax, eax

; 173  : }

	add	rsp, 24
	ret	0
?x86_64_cpu_msi_address@@YA_KPEA_K_KIEE@Z ENDP		; x86_64_cpu_msi_address
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
?x86_64_is_cpu_fxsave_supported@@YA_NXZ PROC		; x86_64_is_cpu_fxsave_supported

; 158  : 	return _fxsave;

	movzx	eax, BYTE PTR ?_fxsave@@3_NA		; _fxsave

; 159  : }

	ret	0
?x86_64_is_cpu_fxsave_supported@@YA_NXZ ENDP		; x86_64_is_cpu_fxsave_supported
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
cr0$ = 48
cr4$1 = 56
cr4$2 = 64
d$ = 72
c$ = 80
b$ = 88
a$ = 96
?x86_64_hal_cpu_feature_enable@@YAXXZ PROC		; x86_64_hal_cpu_feature_enable

; 121  : void x86_64_hal_cpu_feature_enable() {

$LN10:
	sub	rsp, 120				; 00000078H

; 122  : 	uint64_t cr0 = x64_read_cr0();

	call	x64_read_cr0
	mov	QWORD PTR cr0$[rsp], rax

; 123  : 	cr0 &= ~(1 << 2);

	mov	rax, QWORD PTR cr0$[rsp]
	and	rax, -5
	mov	QWORD PTR cr0$[rsp], rax

; 124  : 	cr0 |= (1 << 1);

	mov	rax, QWORD PTR cr0$[rsp]
	or	rax, 2
	mov	QWORD PTR cr0$[rsp], rax

; 125  : 	x64_write_cr0(cr0);

	mov	rcx, QWORD PTR cr0$[rsp]
	call	x64_write_cr0

; 126  : 
; 127  : 	size_t a, b, c, d;
; 128  : 	x64_cpuid(1, &a, &b, &c, &d, 0);

	mov	QWORD PTR [rsp+40], 0
	lea	rax, QWORD PTR d$[rsp]
	mov	QWORD PTR [rsp+32], rax
	lea	r9, QWORD PTR c$[rsp]
	lea	r8, QWORD PTR b$[rsp]
	lea	rdx, QWORD PTR a$[rsp]
	mov	ecx, 1
	call	x64_cpuid

; 129  : 	if ((c & (1 << 26)) != 0) {

	mov	rax, QWORD PTR c$[rsp]
	and	rax, 67108864				; 04000000H
	test	rax, rax
	je	SHORT $LN7@x86_64_hal

; 130  : 		uint64_t cr4 = x64_read_cr4();

	call	x64_read_cr4
	mov	QWORD PTR cr4$2[rsp], rax

; 131  : 		cr4 |= (1 << 18);

	mov	rax, QWORD PTR cr4$2[rsp]
	bts	rax, 18
	mov	QWORD PTR cr4$2[rsp], rax

; 132  : 		x64_write_cr4(cr4);

	mov	rcx, QWORD PTR cr4$2[rsp]
	call	x64_write_cr4
$LN7@x86_64_hal:

; 133  : 	}
; 134  : 
; 135  : 	if ((d & (1 << 25)) != 0) {

	mov	rax, QWORD PTR d$[rsp]
	and	rax, 33554432				; 02000000H
	test	rax, rax
	je	SHORT $LN6@x86_64_hal

; 136  : 		size_t cr4 = x64_read_cr4();

	call	x64_read_cr4
	mov	QWORD PTR cr4$1[rsp], rax

; 137  : 
; 138  : 		if ((d & (1 << 24)) != 0) {

	mov	rax, QWORD PTR d$[rsp]
	and	rax, 16777216				; 01000000H
	test	rax, rax
	je	SHORT $LN5@x86_64_hal

; 139  : 			cr4 |= (1 << 9);

	mov	rax, QWORD PTR cr4$1[rsp]
	bts	rax, 9
	mov	QWORD PTR cr4$1[rsp], rax

; 140  : 			_fxsave = true;

	mov	BYTE PTR ?_fxsave@@3_NA, 1		; _fxsave
$LN5@x86_64_hal:

; 141  : 		}
; 142  : 
; 143  : 		cr4 |= (1 << 10);

	mov	rax, QWORD PTR cr4$1[rsp]
	bts	rax, 10
	mov	QWORD PTR cr4$1[rsp], rax

; 144  : 		x64_write_cr4(cr4);

	mov	rcx, QWORD PTR cr4$1[rsp]
	call	x64_write_cr4
	jmp	SHORT $LN4@x86_64_hal
$LN6@x86_64_hal:

; 145  : 	}
; 146  : 
; 147  : 	else if ((d & (1 << 26)) != 0) {

	mov	rax, QWORD PTR d$[rsp]
	and	rax, 67108864				; 04000000H
	test	rax, rax
	je	SHORT $LN3@x86_64_hal
	jmp	SHORT $LN2@x86_64_hal
$LN3@x86_64_hal:

; 148  : 		//supported SSE2
; 149  : 	}
; 150  : 
; 151  : 	else if ((c & (1 << 0)) != 0) {

	mov	rax, QWORD PTR c$[rsp]
	and	rax, 1
$LN2@x86_64_hal:
$LN4@x86_64_hal:

; 152  : 		//supported SSE3
; 153  : 	}
; 154  : }

	add	rsp, 120				; 00000078H
	ret	0
?x86_64_hal_cpu_feature_enable@@YAXXZ ENDP		; x86_64_hal_cpu_feature_enable
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
?x86_64_get_tss@@YAPEAU_tss@@XZ PROC			; x86_64_get_tss

; 115  : 	return _tss;

	mov	rax, QWORD PTR ?_tss@@3PEAU0@EA		; _tss

; 116  : }

	ret	0
?x86_64_get_tss@@YAPEAU_tss@@XZ ENDP			; x86_64_get_tss
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
code_sel$ = 32
data_sel$ = 36
tss_entry$ = 40
tv64 = 48
tss_$ = 56
peek_gdt$ = 64
bit$ = 96
?x86_64_init_user_ap@@YAX_K@Z PROC			; x86_64_init_user_ap

; 87   : void x86_64_init_user_ap(size_t bit) {

$LN8:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 88					; 00000058H

; 88   : 
; 89   : 	uint16_t data_sel = SEGVAL(GDT_ENTRY_USER_DATA, 3);

	mov	eax, 35					; 00000023H
	mov	WORD PTR data_sel$[rsp], ax

; 90   : 	uint16_t code_sel = 0;

	xor	eax, eax
	mov	WORD PTR code_sel$[rsp], ax

; 91   : 	switch (bit) {

	mov	rax, QWORD PTR bit$[rsp]
	mov	QWORD PTR tv64[rsp], rax
	cmp	QWORD PTR tv64[rsp], 32			; 00000020H
	je	SHORT $LN2@x86_64_ini
	cmp	QWORD PTR tv64[rsp], 64			; 00000040H
	je	SHORT $LN3@x86_64_ini
	jmp	SHORT $LN1@x86_64_ini
$LN3@x86_64_ini:

; 92   : 	case 64:
; 93   : 		code_sel = SEGVAL(GDT_ENTRY_USER_CODE, 3);

	mov	eax, 43					; 0000002bH
	mov	WORD PTR code_sel$[rsp], ax

; 94   : 		break;

	jmp	SHORT $LN4@x86_64_ini
$LN2@x86_64_ini:

; 95   : 	case 32:
; 96   : 		code_sel = SEGVAL(GDT_ENTRY_USER_CODE32, 3);

	mov	eax, 27
	mov	WORD PTR code_sel$[rsp], ax

; 97   : 		break;

	jmp	SHORT $LN4@x86_64_ini
$LN1@x86_64_ini:

; 98   : 	default:
; 99   : 		return;

	jmp	SHORT $LN6@x86_64_ini
$LN4@x86_64_ini:

; 100  : 	}
; 101  : 
; 102  : 	gdtr peek_gdt;
; 103  : 	x64_sgdt(&peek_gdt);

	lea	rcx, QWORD PTR peek_gdt$[rsp]
	call	x64_sgdt

; 104  : 	gdt_entry& tss_entry = peek_gdt.gdtaddr[GDT_ENTRY_TSS];

	mov	eax, 8
	imul	rax, rax, 7
	mov	rcx, QWORD PTR peek_gdt$[rsp+2]
	add	rcx, rax
	mov	rax, rcx
	mov	QWORD PTR tss_entry$[rsp], rax

; 105  : 
; 106  : 	TSS *tss_ = (TSS*)(tss_entry.base_low + (tss_entry.base_mid << 16) + (tss_entry.base_high << 24) + ((uint64_t)*(uint32_t*)&peek_gdt.gdtaddr[GDT_ENTRY_TSS + 1] << 32));

	mov	rax, QWORD PTR tss_entry$[rsp]
	movzx	eax, WORD PTR [rax+2]
	mov	rcx, QWORD PTR tss_entry$[rsp]
	movzx	ecx, BYTE PTR [rcx+4]
	shl	ecx, 16
	add	eax, ecx
	mov	rcx, QWORD PTR tss_entry$[rsp]
	movzx	ecx, BYTE PTR [rcx+7]
	shl	ecx, 24
	add	eax, ecx
	cdqe
	mov	ecx, 8
	imul	rcx, rcx, 8
	mov	rdx, QWORD PTR peek_gdt$[rsp+2]
	mov	ecx, DWORD PTR [rdx+rcx]
	shl	rcx, 32					; 00000020H
	add	rax, rcx
	mov	QWORD PTR tss_$[rsp], rax

; 107  : 
; 108  : 	AuPerCPUSetKernelTSS(tss_);

	mov	rcx, QWORD PTR tss_$[rsp]
	call	?AuPerCPUSetKernelTSS@@YAXPEAU_tss@@@Z	; AuPerCPUSetKernelTSS
$LN6@x86_64_ini:

; 109  : }

	add	rsp, 88					; 00000058H
	ret	0
?x86_64_init_user_ap@@YAX_K@Z ENDP			; x86_64_init_user_ap
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
code_sel$ = 32
data_sel$ = 36
tss_entry$ = 40
tv64 = 48
peek_gdt$ = 56
bit$ = 96
?x86_64_init_user@@YAX_K@Z PROC				; x86_64_init_user

; 59   : void x86_64_init_user(size_t bit) {

$LN8:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 88					; 00000058H

; 60   : 	uint16_t data_sel = SEGVAL(GDT_ENTRY_USER_DATA, 3);

	mov	eax, 35					; 00000023H
	mov	WORD PTR data_sel$[rsp], ax

; 61   : 	uint16_t code_sel = 0;

	xor	eax, eax
	mov	WORD PTR code_sel$[rsp], ax

; 62   : 	switch (bit) {

	mov	rax, QWORD PTR bit$[rsp]
	mov	QWORD PTR tv64[rsp], rax
	cmp	QWORD PTR tv64[rsp], 32			; 00000020H
	je	SHORT $LN2@x86_64_ini
	cmp	QWORD PTR tv64[rsp], 64			; 00000040H
	je	SHORT $LN3@x86_64_ini
	jmp	SHORT $LN1@x86_64_ini
$LN3@x86_64_ini:

; 63   : 	case 64:
; 64   : 		code_sel = SEGVAL(GDT_ENTRY_USER_CODE, 3);

	mov	eax, 43					; 0000002bH
	mov	WORD PTR code_sel$[rsp], ax

; 65   : 		break;

	jmp	SHORT $LN4@x86_64_ini
$LN2@x86_64_ini:

; 66   : 	case 32:
; 67   : 		code_sel = SEGVAL(GDT_ENTRY_USER_CODE32, 3);

	mov	eax, 27
	mov	WORD PTR code_sel$[rsp], ax

; 68   : 		break;

	jmp	SHORT $LN4@x86_64_ini
$LN1@x86_64_ini:

; 69   : 	default:
; 70   : 		return;

	jmp	SHORT $LN6@x86_64_ini
$LN4@x86_64_ini:

; 71   : 	}
; 72   : 
; 73   : 	gdtr peek_gdt;
; 74   : 	x64_sgdt(&peek_gdt);

	lea	rcx, QWORD PTR peek_gdt$[rsp]
	call	x64_sgdt

; 75   : 	gdt_entry& tss_entry = peek_gdt.gdtaddr[GDT_ENTRY_TSS];

	mov	eax, 8
	imul	rax, rax, 7
	mov	rcx, QWORD PTR peek_gdt$[rsp+2]
	add	rcx, rax
	mov	rax, rcx
	mov	QWORD PTR tss_entry$[rsp], rax

; 76   : 
; 77   : 	_tss = (TSS*)(tss_entry.base_low + (tss_entry.base_mid << 16) + (tss_entry.base_high << 24) +
; 78   : 		((uint64_t)*(uint32_t*)&peek_gdt.gdtaddr[GDT_ENTRY_TSS + 1] << 32));

	mov	rax, QWORD PTR tss_entry$[rsp]
	movzx	eax, WORD PTR [rax+2]
	mov	rcx, QWORD PTR tss_entry$[rsp]
	movzx	ecx, BYTE PTR [rcx+4]
	shl	ecx, 16
	add	eax, ecx
	mov	rcx, QWORD PTR tss_entry$[rsp]
	movzx	ecx, BYTE PTR [rcx+7]
	shl	ecx, 24
	add	eax, ecx
	cdqe
	mov	ecx, 8
	imul	rcx, rcx, 8
	mov	rdx, QWORD PTR peek_gdt$[rsp+2]
	mov	ecx, DWORD PTR [rdx+rcx]
	shl	rcx, 32					; 00000020H
	add	rax, rcx
	mov	QWORD PTR ?_tss@@3PEAU0@EA, rax		; _tss
$LN6@x86_64_ini:

; 79   : 
; 80   : }

	add	rsp, 88					; 00000058H
	ret	0
?x86_64_init_user@@YAX_K@Z ENDP				; x86_64_init_user
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\x86_64_cpu.cpp
_TEXT	SEGMENT
efer$ = 32
?x86_64_enable_syscall_ext@@YAXXZ PROC			; x86_64_enable_syscall_ext

; 48   : void x86_64_enable_syscall_ext() {

$LN3:
	sub	rsp, 56					; 00000038H

; 49   : 	size_t efer = x64_read_msr(IA32_EFER);

	mov	ecx, -1073741696			; c0000080H
	call	x64_read_msr
	mov	QWORD PTR efer$[rsp], rax

; 50   : 	efer |= (1 << 11);

	mov	rax, QWORD PTR efer$[rsp]
	bts	rax, 11
	mov	QWORD PTR efer$[rsp], rax

; 51   : 	efer |= 1;

	mov	rax, QWORD PTR efer$[rsp]
	or	rax, 1
	mov	QWORD PTR efer$[rsp], rax

; 52   : 	x64_write_msr(IA32_EFER, efer);

	mov	rdx, QWORD PTR efer$[rsp]
	mov	ecx, -1073741696			; c0000080H
	call	x64_write_msr

; 53   : }

	add	rsp, 56					; 00000038H
	ret	0
?x86_64_enable_syscall_ext@@YAXXZ ENDP			; x86_64_enable_syscall_ext
_TEXT	ENDS
END
