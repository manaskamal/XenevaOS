; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?_IOAPICBase@@3PEAXEA				; _IOAPICBase
_BSS	SEGMENT
?_IOAPICBase@@3PEAXEA DQ 01H DUP (?)			; _IOAPICBase
_BSS	ENDS
PUBLIC	?IOAPICInitialise@@YAXPEAX@Z			; IOAPICInitialise
PUBLIC	?IOAPICMaskIRQ@@YAXE_N@Z			; IOAPICMaskIRQ
PUBLIC	?IOAPICRegisterIRQ@@YAX_KP6AX0PEAX@ZE_N@Z	; IOAPICRegisterIRQ
PUBLIC	?IOAPICRedirect@@YAXEIGE@Z			; IOAPICRedirect
PUBLIC	??$raw_offset@PECIPEAX@@YAPECIPEAXH@Z		; raw_offset<unsigned int volatile * __ptr64,void * __ptr64>
EXTRN	AuMapMMIO:PROC
EXTRN	setvect:PROC
pdata	SEGMENT
$pdata$?IOAPICInitialise@@YAXPEAX@Z DD imagerel $LN6
	DD	imagerel $LN6+158
	DD	imagerel $unwind$?IOAPICInitialise@@YAXPEAX@Z
$pdata$?IOAPICMaskIRQ@@YAXE_N@Z DD imagerel $LN5
	DD	imagerel $LN5+116
	DD	imagerel $unwind$?IOAPICMaskIRQ@@YAXE_N@Z
$pdata$?IOAPICRegisterIRQ@@YAX_KP6AX0PEAX@ZE_N@Z DD imagerel $LN5
	DD	imagerel $LN5+231
	DD	imagerel $unwind$?IOAPICRegisterIRQ@@YAX_KP6AX0PEAX@ZE_N@Z
$pdata$?IOAPICRedirect@@YAXEIGE@Z DD imagerel $LN5
	DD	imagerel $LN5+178
	DD	imagerel $unwind$?IOAPICRedirect@@YAXEIGE@Z
$pdata$?ReadIOAPICRegister@@YAIE@Z DD imagerel ?ReadIOAPICRegister@@YAIE@Z
	DD	imagerel ?ReadIOAPICRegister@@YAIE@Z+66
	DD	imagerel $unwind$?ReadIOAPICRegister@@YAIE@Z
$pdata$?WriteIOAPICRegister@@YAXEI@Z DD imagerel ?WriteIOAPICRegister@@YAXEI@Z
	DD	imagerel ?WriteIOAPICRegister@@YAXEI@Z+74
	DD	imagerel $unwind$?WriteIOAPICRegister@@YAXEI@Z
pdata	ENDS
xdata	SEGMENT
$unwind$?IOAPICInitialise@@YAXPEAX@Z DD 010901H
	DD	08209H
$unwind$?IOAPICMaskIRQ@@YAXE_N@Z DD 010c01H
	DD	0620cH
$unwind$?IOAPICRegisterIRQ@@YAX_KP6AX0PEAX@ZE_N@Z DD 011801H
	DD	06218H
$unwind$?IOAPICRedirect@@YAXEIGE@Z DD 011701H
	DD	06217H
$unwind$?ReadIOAPICRegister@@YAIE@Z DD 010801H
	DD	06208H
$unwind$?WriteIOAPICRegister@@YAXEI@Z DD 010c01H
	DD	0620cH
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\ioapic.cpp
_TEXT	SEGMENT
ioregsel$ = 32
ioregwin$ = 40
offset$ = 64
val$ = 72
?WriteIOAPICRegister@@YAXEI@Z PROC			; WriteIOAPICRegister

; 53   : static void WriteIOAPICRegister(const uint8_t offset, const uint32_t val) {

	mov	DWORD PTR [rsp+16], edx
	mov	BYTE PTR [rsp+8], cl
	sub	rsp, 56					; 00000038H

; 54   : 	volatile uint32_t* ioregsel = reinterpret_cast<volatile uint32_t*>(_IOAPICBase);

	mov	rax, QWORD PTR ?_IOAPICBase@@3PEAXEA	; _IOAPICBase
	mov	QWORD PTR ioregsel$[rsp], rax

; 55   : 	volatile uint32_t* ioregwin = raw_offset<volatile uint32_t*>(_IOAPICBase, 0x10);

	mov	edx, 16
	mov	rcx, QWORD PTR ?_IOAPICBase@@3PEAXEA	; _IOAPICBase
	call	??$raw_offset@PECIPEAX@@YAPECIPEAXH@Z	; raw_offset<unsigned int volatile * __ptr64,void * __ptr64>
	mov	QWORD PTR ioregwin$[rsp], rax

; 56   : 	*ioregsel = offset;

	movzx	eax, BYTE PTR offset$[rsp]
	mov	rcx, QWORD PTR ioregsel$[rsp]
	mov	DWORD PTR [rcx], eax

; 57   : 	*ioregwin = val;

	mov	rax, QWORD PTR ioregwin$[rsp]
	mov	ecx, DWORD PTR val$[rsp]
	mov	DWORD PTR [rax], ecx

; 58   : }

	add	rsp, 56					; 00000038H
	ret	0
?WriteIOAPICRegister@@YAXEI@Z ENDP			; WriteIOAPICRegister
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\basehdr\stdint.h
;	COMDAT ??$raw_offset@PECIPEAX@@YAPECIPEAXH@Z
_TEXT	SEGMENT
p1$ = 8
offset$ = 16
??$raw_offset@PECIPEAX@@YAPECIPEAXH@Z PROC		; raw_offset<unsigned int volatile * __ptr64,void * __ptr64>, COMDAT

; 207  : 	{

	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx

; 208  : 		return (T)((size_t)p1 + offset);

	movsxd	rax, DWORD PTR offset$[rsp]
	mov	rcx, QWORD PTR p1$[rsp]
	add	rcx, rax
	mov	rax, rcx

; 209  : 	};

	ret	0
??$raw_offset@PECIPEAX@@YAPECIPEAXH@Z ENDP		; raw_offset<unsigned int volatile * __ptr64,void * __ptr64>
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\ioapic.cpp
_TEXT	SEGMENT
ioregsel$ = 32
ioregwin$ = 40
offset$ = 64
?ReadIOAPICRegister@@YAIE@Z PROC			; ReadIOAPICRegister

; 41   : static uint32_t ReadIOAPICRegister(const uint8_t offset) {

	mov	BYTE PTR [rsp+8], cl
	sub	rsp, 56					; 00000038H

; 42   : 	volatile uint32_t* ioregsel = reinterpret_cast<volatile uint32_t*>(_IOAPICBase);

	mov	rax, QWORD PTR ?_IOAPICBase@@3PEAXEA	; _IOAPICBase
	mov	QWORD PTR ioregsel$[rsp], rax

; 43   : 	volatile uint32_t* ioregwin = raw_offset<volatile uint32_t*>(_IOAPICBase, 0x10);

	mov	edx, 16
	mov	rcx, QWORD PTR ?_IOAPICBase@@3PEAXEA	; _IOAPICBase
	call	??$raw_offset@PECIPEAX@@YAPECIPEAXH@Z	; raw_offset<unsigned int volatile * __ptr64,void * __ptr64>
	mov	QWORD PTR ioregwin$[rsp], rax

; 44   : 	*ioregsel = offset;

	movzx	eax, BYTE PTR offset$[rsp]
	mov	rcx, QWORD PTR ioregsel$[rsp]
	mov	DWORD PTR [rcx], eax

; 45   : 	return *ioregwin;

	mov	rax, QWORD PTR ioregwin$[rsp]
	mov	eax, DWORD PTR [rax]

; 46   : }

	add	rsp, 56					; 00000038H
	ret	0
?ReadIOAPICRegister@@YAIE@Z ENDP			; ReadIOAPICRegister
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\ioapic.cpp
_TEXT	SEGMENT
ioredtbl$ = 32
ioapic_base$ = 36
redirection$ = 40
irq$ = 64
gsi$ = 72
flags$ = 80
apic$ = 88
?IOAPICRedirect@@YAXEIGE@Z PROC				; IOAPICRedirect

; 90   : void IOAPICRedirect(uint8_t irq, uint32_t gsi, uint16_t flags, uint8_t apic) {

$LN5:
	mov	BYTE PTR [rsp+32], r9b
	mov	WORD PTR [rsp+24], r8w
	mov	DWORD PTR [rsp+16], edx
	mov	BYTE PTR [rsp+8], cl
	sub	rsp, 56					; 00000038H

; 91   : 	uint32_t ioapic_base = 0xfec00000;

	mov	DWORD PTR ioapic_base$[rsp], -20971520	; fec00000H

; 92   : 	uint64_t redirection = irq + 32;

	movzx	eax, BYTE PTR irq$[rsp]
	add	eax, 32					; 00000020H
	cdqe
	mov	QWORD PTR redirection$[rsp], rax

; 93   : 
; 94   : 	if (flags & 2)

	movzx	eax, WORD PTR flags$[rsp]
	and	eax, 2
	test	eax, eax
	je	SHORT $LN2@IOAPICRedi

; 95   : 		redirection |= 1 << 13;

	mov	rax, QWORD PTR redirection$[rsp]
	bts	rax, 13
	mov	QWORD PTR redirection$[rsp], rax
$LN2@IOAPICRedi:

; 96   : 	
; 97   : 	if (flags & 8)

	movzx	eax, WORD PTR flags$[rsp]
	and	eax, 8
	test	eax, eax
	je	SHORT $LN1@IOAPICRedi

; 98   : 		redirection |= 1 << 15;

	mov	rax, QWORD PTR redirection$[rsp]
	bts	rax, 15
	mov	QWORD PTR redirection$[rsp], rax
$LN1@IOAPICRedi:

; 99   : 
; 100  : 	redirection |= ((uint64_t)apic) << 56;

	movzx	eax, BYTE PTR apic$[rsp]
	shl	rax, 56					; 00000038H
	mov	rcx, QWORD PTR redirection$[rsp]
	or	rcx, rax
	mov	rax, rcx
	mov	QWORD PTR redirection$[rsp], rax

; 101  : 
; 102  : 	uint32_t ioredtbl = (gsi - 0) * 2 + 16;

	mov	eax, DWORD PTR gsi$[rsp]
	lea	eax, DWORD PTR [rax+rax+16]
	mov	DWORD PTR ioredtbl$[rsp], eax

; 103  : 
; 104  : 	WriteIOAPICRegister(ioredtbl + 0, (uint32_t)redirection);

	mov	edx, DWORD PTR redirection$[rsp]
	movzx	ecx, BYTE PTR ioredtbl$[rsp]
	call	?WriteIOAPICRegister@@YAXEI@Z		; WriteIOAPICRegister

; 105  : 	WriteIOAPICRegister(ioredtbl + 1, (uint32_t)(redirection >> 32));

	mov	rax, QWORD PTR redirection$[rsp]
	shr	rax, 32					; 00000020H
	mov	ecx, DWORD PTR ioredtbl$[rsp]
	inc	ecx
	mov	edx, eax
	call	?WriteIOAPICRegister@@YAXEI@Z		; WriteIOAPICRegister

; 106  : 
; 107  : }

	add	rsp, 56					; 00000038H
	ret	0
?IOAPICRedirect@@YAXEIGE@Z ENDP				; IOAPICRedirect
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\ioapic.cpp
_TEXT	SEGMENT
low$ = 32
high_reg$ = 36
reg$ = 40
vector$ = 64
fn$ = 72
irq$ = 80
level$ = 88
?IOAPICRegisterIRQ@@YAX_KP6AX0PEAX@ZE_N@Z PROC		; IOAPICRegisterIRQ

; 67   : void IOAPICRegisterIRQ(size_t vector, void(*fn)(size_t, void*), uint8_t irq, bool level) {

$LN5:
	mov	BYTE PTR [rsp+32], r9b
	mov	BYTE PTR [rsp+24], r8b
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 68   : 	uint32_t reg = IOAPIC_REG_RED_TBL_BASE + irq * 2;

	movzx	eax, BYTE PTR irq$[rsp]
	lea	eax, DWORD PTR [rax+rax+16]
	mov	DWORD PTR reg$[rsp], eax

; 69   : 	uint32_t high_reg = ReadIOAPICRegister(reg + 1);

	mov	eax, DWORD PTR reg$[rsp]
	inc	eax
	movzx	ecx, al
	call	?ReadIOAPICRegister@@YAIE@Z		; ReadIOAPICRegister
	mov	DWORD PTR high_reg$[rsp], eax

; 70   : 	high_reg &= ~0xff000000;

	mov	eax, DWORD PTR high_reg$[rsp]
	and	eax, 16777215				; 00ffffffH
	mov	DWORD PTR high_reg$[rsp], eax

; 71   : 	high_reg |= (0 << 24);

	mov	eax, DWORD PTR high_reg$[rsp]
	mov	DWORD PTR high_reg$[rsp], eax

; 72   : 	WriteIOAPICRegister(reg + 1, high_reg);

	mov	eax, DWORD PTR reg$[rsp]
	inc	eax
	mov	edx, DWORD PTR high_reg$[rsp]
	movzx	ecx, al
	call	?WriteIOAPICRegister@@YAXEI@Z		; WriteIOAPICRegister

; 73   : 	uint32_t low = ReadIOAPICRegister(reg);

	movzx	ecx, BYTE PTR reg$[rsp]
	call	?ReadIOAPICRegister@@YAIE@Z		; ReadIOAPICRegister
	mov	DWORD PTR low$[rsp], eax

; 74   : 
; 75   : 	low &= ~(1 << 16);

	mov	eax, DWORD PTR low$[rsp]
	btr	eax, 16
	mov	DWORD PTR low$[rsp], eax

; 76   : 
; 77   : 	if (level)

	movzx	eax, BYTE PTR level$[rsp]
	test	eax, eax
	je	SHORT $LN2@IOAPICRegi

; 78   : 		low |= (1 << 15);

	mov	eax, DWORD PTR low$[rsp]
	bts	eax, 15
	mov	DWORD PTR low$[rsp], eax

; 79   : 	else

	jmp	SHORT $LN1@IOAPICRegi
$LN2@IOAPICRegi:

; 80   : 		low |= (0 << 15);

	mov	eax, DWORD PTR low$[rsp]
	mov	DWORD PTR low$[rsp], eax
$LN1@IOAPICRegi:

; 81   : 	
; 82   : 	low |= (0 << 13);

	mov	eax, DWORD PTR low$[rsp]
	mov	DWORD PTR low$[rsp], eax

; 83   : 	low |= (0 << 11);

	mov	eax, DWORD PTR low$[rsp]
	mov	DWORD PTR low$[rsp], eax

; 84   : 	low |= vector + 32;

	mov	rax, QWORD PTR vector$[rsp]
	add	rax, 32					; 00000020H
	mov	ecx, DWORD PTR low$[rsp]
	or	rcx, rax
	mov	rax, rcx
	mov	DWORD PTR low$[rsp], eax

; 85   : 	WriteIOAPICRegister(reg, low);

	mov	edx, DWORD PTR low$[rsp]
	movzx	ecx, BYTE PTR reg$[rsp]
	call	?WriteIOAPICRegister@@YAXEI@Z		; WriteIOAPICRegister

; 86   : 	setvect(vector + 32, fn);

	mov	rax, QWORD PTR vector$[rsp]
	add	rax, 32					; 00000020H
	mov	rdx, QWORD PTR fn$[rsp]
	mov	rcx, rax
	call	setvect

; 87   : }

	add	rsp, 56					; 00000038H
	ret	0
?IOAPICRegisterIRQ@@YAX_KP6AX0PEAX@ZE_N@Z ENDP		; IOAPICRegisterIRQ
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\ioapic.cpp
_TEXT	SEGMENT
low$ = 32
reg$ = 36
irq$ = 64
value$ = 72
?IOAPICMaskIRQ@@YAXE_N@Z PROC				; IOAPICMaskIRQ

; 115  : void IOAPICMaskIRQ(uint8_t irq, bool value) {

$LN5:
	mov	BYTE PTR [rsp+16], dl
	mov	BYTE PTR [rsp+8], cl
	sub	rsp, 56					; 00000038H

; 116  : 	uint32_t reg = IOAPIC_REG_RED_TBL_BASE + irq * 2;

	movzx	eax, BYTE PTR irq$[rsp]
	lea	eax, DWORD PTR [rax+rax+16]
	mov	DWORD PTR reg$[rsp], eax

; 117  : 	WriteIOAPICRegister(reg + 1, ReadIOAPICRegister(0x02) << 24);

	mov	cl, 2
	call	?ReadIOAPICRegister@@YAIE@Z		; ReadIOAPICRegister
	shl	eax, 24
	mov	ecx, DWORD PTR reg$[rsp]
	inc	ecx
	mov	edx, eax
	call	?WriteIOAPICRegister@@YAXEI@Z		; WriteIOAPICRegister

; 118  : 	uint32_t low = ReadIOAPICRegister(reg);

	movzx	ecx, BYTE PTR reg$[rsp]
	call	?ReadIOAPICRegister@@YAIE@Z		; ReadIOAPICRegister
	mov	DWORD PTR low$[rsp], eax

; 119  : 
; 120  : 	if (value)

	movzx	eax, BYTE PTR value$[rsp]
	test	eax, eax
	je	SHORT $LN2@IOAPICMask

; 121  : 		low |= (1 << 16);

	mov	eax, DWORD PTR low$[rsp]
	bts	eax, 16
	mov	DWORD PTR low$[rsp], eax

; 122  : 	else

	jmp	SHORT $LN1@IOAPICMask
$LN2@IOAPICMask:

; 123  : 		low &= ~(1 << 16);

	mov	eax, DWORD PTR low$[rsp]
	btr	eax, 16
	mov	DWORD PTR low$[rsp], eax
$LN1@IOAPICMask:

; 124  : 
; 125  : 	WriteIOAPICRegister(reg, low);

	mov	edx, DWORD PTR low$[rsp]
	movzx	ecx, BYTE PTR reg$[rsp]
	call	?WriteIOAPICRegister@@YAXEI@Z		; WriteIOAPICRegister

; 126  : }

	add	rsp, 56					; 00000038H
	ret	0
?IOAPICMaskIRQ@@YAXE_N@Z ENDP				; IOAPICMaskIRQ
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\ioapic.cpp
_TEXT	SEGMENT
reg$1 = 32
ver$ = 36
intr_num$ = 40
val$2 = 44
n$3 = 48
ioapic_phys$ = 56
address$ = 80
?IOAPICInitialise@@YAXPEAX@Z PROC			; IOAPICInitialise

; 132  : void IOAPICInitialise(void* address) {

$LN6:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 72					; 00000048H

; 133  : 	uint64_t ioapic_phys = (uint64_t)address;

	mov	rax, QWORD PTR address$[rsp]
	mov	QWORD PTR ioapic_phys$[rsp], rax

; 134  : 	_IOAPICBase = (void*)AuMapMMIO(ioapic_phys, 2);

	mov	edx, 2
	mov	rcx, QWORD PTR ioapic_phys$[rsp]
	call	AuMapMMIO
	mov	QWORD PTR ?_IOAPICBase@@3PEAXEA, rax	; _IOAPICBase

; 135  : 
; 136  : 	uint32_t ver = ReadIOAPICRegister(IOAPIC_REG_VER);

	mov	cl, 1
	call	?ReadIOAPICRegister@@YAIE@Z		; ReadIOAPICRegister
	mov	DWORD PTR ver$[rsp], eax

; 137  : 	uint32_t intr_num = (ver >> 16) & 0xff;

	mov	eax, DWORD PTR ver$[rsp]
	shr	eax, 16
	and	eax, 255				; 000000ffH
	mov	DWORD PTR intr_num$[rsp], eax

; 138  : 	for (size_t n = 0; n <= intr_num; ++n) {

	mov	QWORD PTR n$3[rsp], 0
	jmp	SHORT $LN3@IOAPICInit
$LN2@IOAPICInit:
	mov	rax, QWORD PTR n$3[rsp]
	inc	rax
	mov	QWORD PTR n$3[rsp], rax
$LN3@IOAPICInit:
	mov	eax, DWORD PTR intr_num$[rsp]
	cmp	QWORD PTR n$3[rsp], rax
	ja	SHORT $LN1@IOAPICInit

; 139  : 		uint32_t reg = IOAPIC_REG_RED_TBL_BASE + n * 2;

	mov	rax, QWORD PTR n$3[rsp]
	lea	rax, QWORD PTR [rax+rax+16]
	mov	DWORD PTR reg$1[rsp], eax

; 140  : 		uint32_t val = ReadIOAPICRegister(reg);

	movzx	ecx, BYTE PTR reg$1[rsp]
	call	?ReadIOAPICRegister@@YAIE@Z		; ReadIOAPICRegister
	mov	DWORD PTR val$2[rsp], eax

; 141  : 		WriteIOAPICRegister(reg, val | (1 << 16));

	mov	eax, DWORD PTR val$2[rsp]
	bts	eax, 16
	mov	edx, eax
	movzx	ecx, BYTE PTR reg$1[rsp]
	call	?WriteIOAPICRegister@@YAXEI@Z		; WriteIOAPICRegister

; 142  : 	}

	jmp	SHORT $LN2@IOAPICInit
$LN1@IOAPICInit:

; 143  : }

	add	rsp, 72					; 00000048H
	ret	0
?IOAPICInitialise@@YAXPEAX@Z ENDP			; IOAPICInitialise
_TEXT	ENDS
END
