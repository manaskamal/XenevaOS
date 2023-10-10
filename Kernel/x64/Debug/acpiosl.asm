; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

CONST	SEGMENT
$SG8588	DB	'Freeing up ', 0aH, 00H
	ORG $+3
$SG8699	DB	'ACPI interrupt handler called', 0aH, 00H
	ORG $+1
$SG8748	DB	'read mem', 0aH, 00H
	ORG $+6
$SG8769	DB	'Write mem', 0aH, 00H
	ORG $+5
$SG8790	DB	'read port', 0aH, 00H
	ORG $+5
$SG8804	DB	'Write port', 0aH, 00H
CONST	ENDS
PUBLIC	AcpiOsAcquireGlobalLock
PUBLIC	AcpiOsReleaseGlobalLock
PUBLIC	AcpiOsInitialize
PUBLIC	AcpiOsTerminate
PUBLIC	AcpiOsGetRootPointer
PUBLIC	AcpiOsPredefinedOverride
PUBLIC	AcpiOsTableOverride
PUBLIC	AcpiOsPhysicalTableOverride
PUBLIC	AcpiOsCreateLock
PUBLIC	AcpiOsDeleteLock
PUBLIC	AcpiOsAcquireLock
PUBLIC	AcpiOsReleaseLock
PUBLIC	AcpiOsCreateSemaphore
PUBLIC	AcpiOsDeleteSemaphore
PUBLIC	AcpiOsWaitSemaphore
PUBLIC	AcpiOsSignalSemaphore
PUBLIC	AcpiOsCreateMutex
PUBLIC	AcpiOsDeleteMutex
PUBLIC	AcpiOsAcquireMutex
PUBLIC	AcpiOsReleaseMutex
PUBLIC	AcpiOsAllocate
PUBLIC	AcpiOsFree
PUBLIC	AcpiOsMapMemory
PUBLIC	AcpiOsUnmapMemory
PUBLIC	AcpiOsGetPhysicalAddress
PUBLIC	AcpiOsInstallInterruptHandler
PUBLIC	AcpiOsRemoveInterruptHandler
PUBLIC	AcpiOsGetThreadId
PUBLIC	AcpiOsExecute
PUBLIC	AcpiOsWaitEventsComplete
PUBLIC	AcpiOsSleep
PUBLIC	AcpiOsStall
PUBLIC	AcpiOsReadPort
PUBLIC	AcpiOsWritePort
PUBLIC	AcpiOsReadMemory
PUBLIC	AcpiOsWriteMemory
PUBLIC	AcpiOsReadPciConfiguration
PUBLIC	AcpiOsWritePciConfiguration
PUBLIC	AcpiOsReadable
PUBLIC	AcpiOsWritable
PUBLIC	AcpiOsGetTimer
PUBLIC	AcpiOsSignal
PUBLIC	AcpiOsEnterSleep
PUBLIC	AcpiOsPrintf
PUBLIC	AcpiOsVprintf
EXTRN	AuMapPage:PROC
EXTRN	AuGetFreePage:PROC
EXTRN	AuFreePages:PROC
EXTRN	AuGetPhysicalAddress:PROC
EXTRN	kmalloc:PROC
EXTRN	kfree:PROC
EXTRN	?AuACPIGetRSDP@@YAPEAXXZ:PROC			; AuACPIGetRSDP
EXTRN	AuPCIEWrite:PROC
EXTRN	AuPCIERead:PROC
EXTRN	AuTextOut:PROC
EXTRN	x64_inportd:PROC
EXTRN	x64_outportd:PROC
pdata	SEGMENT
$pdata$AcpiOsGetRootPointer DD imagerel $LN3
	DD	imagerel $LN3+24
	DD	imagerel $unwind$AcpiOsGetRootPointer
$pdata$AcpiOsCreateLock DD imagerel $LN3
	DD	imagerel $LN3+52
	DD	imagerel $unwind$AcpiOsCreateLock
$pdata$AcpiOsCreateMutex DD imagerel $LN3
	DD	imagerel $LN3+52
	DD	imagerel $unwind$AcpiOsCreateMutex
$pdata$AcpiOsDeleteMutex DD imagerel $LN3
	DD	imagerel $LN3+24
	DD	imagerel $unwind$AcpiOsDeleteMutex
$pdata$AcpiOsAllocate DD imagerel $LN3
	DD	imagerel $LN3+23
	DD	imagerel $unwind$AcpiOsAllocate
$pdata$AcpiOsFree DD imagerel $LN3
	DD	imagerel $LN3+36
	DD	imagerel $unwind$AcpiOsFree
$pdata$AcpiOsMapMemory DD imagerel $LN6
	DD	imagerel $LN6+198
	DD	imagerel $unwind$AcpiOsMapMemory
$pdata$AcpiOsUnmapMemory DD imagerel $LN3
	DD	imagerel $LN3+36
	DD	imagerel $unwind$AcpiOsUnmapMemory
$pdata$AcpiOsGetPhysicalAddress DD imagerel $LN5
	DD	imagerel $LN5+72
	DD	imagerel $unwind$AcpiOsGetPhysicalAddress
$pdata$AcpiOsExecute DD imagerel $LN3
	DD	imagerel $LN3+34
	DD	imagerel $unwind$AcpiOsExecute
$pdata$AcpiOsSleep DD imagerel $LN6
	DD	imagerel $LN6+44
	DD	imagerel $unwind$AcpiOsSleep
$pdata$AcpiOsStall DD imagerel $LN6
	DD	imagerel $LN6+41
	DD	imagerel $unwind$AcpiOsStall
$pdata$AcpiOsReadPort DD imagerel $LN7
	DD	imagerel $LN7+95
	DD	imagerel $unwind$AcpiOsReadPort
$pdata$AcpiOsWritePort DD imagerel $LN7
	DD	imagerel $LN7+91
	DD	imagerel $unwind$AcpiOsWritePort
$pdata$AcpiOsReadMemory DD imagerel $LN10
	DD	imagerel $LN10+154
	DD	imagerel $unwind$AcpiOsReadMemory
$pdata$AcpiOsWriteMemory DD imagerel $LN10
	DD	imagerel $LN10+140
	DD	imagerel $unwind$AcpiOsWriteMemory
$pdata$AcpiOsReadPciConfiguration DD imagerel $LN4
	DD	imagerel $LN4+137
	DD	imagerel $unwind$AcpiOsReadPciConfiguration
$pdata$AcpiOsWritePciConfiguration DD imagerel $LN3
	DD	imagerel $LN3+105
	DD	imagerel $unwind$AcpiOsWritePciConfiguration
$pdata$AcpiOsPrintf DD imagerel $LN3
	DD	imagerel $LN3+49
	DD	imagerel $unwind$AcpiOsPrintf
$pdata$AcpiOsVprintf DD imagerel $LN3
	DD	imagerel $LN3+29
	DD	imagerel $unwind$AcpiOsVprintf
pdata	ENDS
xdata	SEGMENT
$unwind$AcpiOsGetRootPointer DD 010401H
	DD	06204H
$unwind$AcpiOsCreateLock DD 010901H
	DD	06209H
$unwind$AcpiOsCreateMutex DD 010901H
	DD	06209H
$unwind$AcpiOsDeleteMutex DD 010901H
	DD	04209H
$unwind$AcpiOsAllocate DD 010901H
	DD	04209H
$unwind$AcpiOsFree DD 010901H
	DD	04209H
$unwind$AcpiOsMapMemory DD 010e01H
	DD	0a20eH
$unwind$AcpiOsUnmapMemory DD 010e01H
	DD	0420eH
$unwind$AcpiOsGetPhysicalAddress DD 010e01H
	DD	0620eH
$unwind$AcpiOsExecute DD 011201H
	DD	04212H
$unwind$AcpiOsSleep DD 010901H
	DD	02209H
$unwind$AcpiOsStall DD 010801H
	DD	02208H
$unwind$AcpiOsReadPort DD 011301H
	DD	06213H
$unwind$AcpiOsWritePort DD 011201H
	DD	06212H
$unwind$AcpiOsReadMemory DD 011301H
	DD	06213H
$unwind$AcpiOsWriteMemory DD 011301H
	DD	06213H
$unwind$AcpiOsReadPciConfiguration DD 011701H
	DD	08217H
$unwind$AcpiOsWritePciConfiguration DD 011701H
	DD	08217H
$unwind$AcpiOsPrintf DD 011801H
	DD	06218H
$unwind$AcpiOsVprintf DD 010e01H
	DD	0420eH
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Format$ = 48
Args$ = 56
AcpiOsVprintf PROC

; 233  : {

$LN3:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 234  : 	AuTextOut(Format);

	mov	rcx, QWORD PTR Format$[rsp]
	call	AuTextOut

; 235  : }

	add	rsp, 40					; 00000028H
	ret	0
AcpiOsVprintf ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
args$ = 32
Format$ = 64
AcpiOsPrintf PROC

; 225  : {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+32], r9
	sub	rsp, 56					; 00000038H

; 226  : 	va_list args;
; 227  : 	va_start(args, Format);

	lea	rax, QWORD PTR Format$[rsp+8]
	mov	QWORD PTR args$[rsp], rax

; 228  : 	AuTextOut(Format);

	mov	rcx, QWORD PTR Format$[rsp]
	call	AuTextOut

; 229  : 	va_end(args);
; 230  : }

	add	rsp, 56					; 00000038H
	ret	0
AcpiOsPrintf ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
SleepState$ = 8
RegaValue$ = 16
RegbValue$ = 24
AcpiOsEnterSleep PROC

; 256  : {

	mov	DWORD PTR [rsp+24], r8d
	mov	DWORD PTR [rsp+16], edx
	mov	BYTE PTR [rsp+8], cl

; 257  : 	return AE_NOT_IMPLEMENTED;

	mov	eax, 14

; 258  : }

	ret	0
AcpiOsEnterSleep ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Function$ = 8
Info$ = 16
AcpiOsSignal PROC

; 247  : {

	mov	QWORD PTR [rsp+16], rdx
	mov	DWORD PTR [rsp+8], ecx

; 248  : 	return AE_NOT_FOUND;

	mov	eax, 5

; 249  : }

	ret	0
AcpiOsSignal ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
AcpiOsGetTimer PROC

; 356  : 	return 0; // sys_timer() * 10000;

	xor	eax, eax

; 357  : }

	ret	0
AcpiOsGetTimer ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
memory$ = 8
Length$ = 16
AcpiOsWritable PROC

; 107  : AU_EXTERN AU_FUNC BOOLEAN AcpiOsWritable(void* memory, ACPI_SIZE Length) {

	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx

; 108  : 	return TRUE;

	mov	al, 1

; 109  : }

	ret	0
AcpiOsWritable ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
memory$ = 8
Length$ = 16
AcpiOsReadable PROC

; 103  : AU_EXTERN AU_FUNC BOOLEAN AcpiOsReadable(void *memory, ACPI_SIZE Length) {

	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx

; 104  : 	return TRUE;

	mov	al, 1

; 105  : }

	ret	0
AcpiOsReadable ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
tv76 = 48
PciId$ = 80
Reg$ = 88
Value$ = 96
Width$ = 104
AcpiOsWritePciConfiguration PROC

; 384  : {

$LN3:
	mov	DWORD PTR [rsp+32], r9d
	mov	QWORD PTR [rsp+24], r8
	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 72					; 00000048H

; 385  : 	AuPCIEWrite(PciId->Device, Reg, Value, PciId->Bus, PciId->Device, PciId->Function);

	mov	rax, QWORD PTR PciId$[rsp]
	movzx	eax, WORD PTR [rax+6]
	mov	rcx, QWORD PTR PciId$[rsp]
	movzx	ecx, WORD PTR [rcx+4]
	mov	rdx, QWORD PTR PciId$[rsp]
	movzx	edx, WORD PTR [rdx+2]
	mov	r8, QWORD PTR PciId$[rsp]
	movzx	r8d, WORD PTR [r8+4]
	mov	QWORD PTR tv76[rsp], r8
	mov	DWORD PTR [rsp+40], eax
	mov	DWORD PTR [rsp+32], ecx
	mov	r9d, edx
	mov	r8d, DWORD PTR Value$[rsp]
	mov	edx, DWORD PTR Reg$[rsp]
	mov	rax, QWORD PTR tv76[rsp]
	mov	rcx, rax
	call	AuPCIEWrite

; 386  : 	return AE_OK;

	xor	eax, eax

; 387  : }

	add	rsp, 72					; 00000048H
	ret	0
AcpiOsWritePciConfiguration ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
result$ = 48
tv76 = 56
PciId$ = 80
Reg$ = 88
Value$ = 96
Width$ = 104
AcpiOsReadPciConfiguration PROC

; 369  : {

$LN4:
	mov	DWORD PTR [rsp+32], r9d
	mov	QWORD PTR [rsp+24], r8
	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 72					; 00000048H

; 370  : 	uint32_t result = 0;

	mov	DWORD PTR result$[rsp], 0

; 371  : 	result = AuPCIERead(PciId->Device, Reg, PciId->Bus, PciId->Device, PciId->Function);

	mov	rax, QWORD PTR PciId$[rsp]
	movzx	eax, WORD PTR [rax+6]
	mov	rcx, QWORD PTR PciId$[rsp]
	movzx	ecx, WORD PTR [rcx+4]
	mov	rdx, QWORD PTR PciId$[rsp]
	movzx	edx, WORD PTR [rdx+2]
	mov	r8, QWORD PTR PciId$[rsp]
	movzx	r8d, WORD PTR [r8+4]
	mov	QWORD PTR tv76[rsp], r8
	mov	DWORD PTR [rsp+32], eax
	mov	r9d, ecx
	mov	r8d, edx
	mov	edx, DWORD PTR Reg$[rsp]
	mov	rax, QWORD PTR tv76[rsp]
	mov	rcx, rax
	call	AuPCIERead
	mov	DWORD PTR result$[rsp], eax

; 372  : 	*Value = result;

	mov	eax, DWORD PTR result$[rsp]
	mov	rcx, QWORD PTR Value$[rsp]
	mov	QWORD PTR [rcx], rax

; 373  : 	if (!result)

	cmp	DWORD PTR result$[rsp], 0
	jne	SHORT $LN1@AcpiOsRead

; 374  : 		return AE_NOT_IMPLEMENTED;

	mov	eax, 14
	jmp	SHORT $LN2@AcpiOsRead
$LN1@AcpiOsRead:

; 375  : 	return AE_OK;

	xor	eax, eax
$LN2@AcpiOsRead:

; 376  : }

	add	rsp, 72					; 00000048H
	ret	0
AcpiOsReadPciConfiguration ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
tv65 = 32
Address$ = 64
Value$ = 72
Width$ = 80
AcpiOsWriteMemory PROC

; 292  : {

$LN10:
	mov	DWORD PTR [rsp+24], r8d
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 293  : 	AuTextOut("Write mem\n");

	lea	rcx, OFFSET FLAT:$SG8769
	call	AuTextOut

; 294  : 	switch (Width)

	mov	eax, DWORD PTR Width$[rsp]
	mov	DWORD PTR tv65[rsp], eax
	cmp	DWORD PTR tv65[rsp], 8
	je	SHORT $LN5@AcpiOsWrit
	cmp	DWORD PTR tv65[rsp], 16
	je	SHORT $LN4@AcpiOsWrit
	cmp	DWORD PTR tv65[rsp], 32			; 00000020H
	je	SHORT $LN3@AcpiOsWrit
	cmp	DWORD PTR tv65[rsp], 64			; 00000040H
	je	SHORT $LN2@AcpiOsWrit
	jmp	SHORT $LN1@AcpiOsWrit
$LN5@AcpiOsWrit:

; 295  : 	{
; 296  : 	case 8:
; 297  : 		*(UINT8*)Address = Value;

	mov	rax, QWORD PTR Address$[rsp]
	movzx	ecx, BYTE PTR Value$[rsp]
	mov	BYTE PTR [rax], cl

; 298  : 		break;

	jmp	SHORT $LN6@AcpiOsWrit
$LN4@AcpiOsWrit:

; 299  : 	case 16:
; 300  : 		*(UINT16*)Address = Value;

	mov	rax, QWORD PTR Address$[rsp]
	movzx	ecx, WORD PTR Value$[rsp]
	mov	WORD PTR [rax], cx

; 301  : 		break;

	jmp	SHORT $LN6@AcpiOsWrit
$LN3@AcpiOsWrit:

; 302  : 	case 32:
; 303  : 		*(UINT32*)Address = Value;

	mov	rax, QWORD PTR Address$[rsp]
	mov	ecx, DWORD PTR Value$[rsp]
	mov	DWORD PTR [rax], ecx

; 304  : 		break;

	jmp	SHORT $LN6@AcpiOsWrit
$LN2@AcpiOsWrit:

; 305  : 	case 64:
; 306  : 		*(UINT64*)Address = Value;

	mov	rax, QWORD PTR Address$[rsp]
	mov	rcx, QWORD PTR Value$[rsp]
	mov	QWORD PTR [rax], rcx

; 307  : 		break;

	jmp	SHORT $LN6@AcpiOsWrit
$LN1@AcpiOsWrit:

; 308  : 	default:
; 309  : 		return AE_BAD_VALUE;

	mov	eax, 8196				; 00002004H
	jmp	SHORT $LN8@AcpiOsWrit
$LN6@AcpiOsWrit:

; 310  : 	}
; 311  : 	return AE_OK;

	xor	eax, eax
$LN8@AcpiOsWrit:

; 312  : }

	add	rsp, 56					; 00000038H
	ret	0
AcpiOsWriteMemory ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
tv65 = 32
Address$ = 64
Value$ = 72
Width$ = 80
AcpiOsReadMemory PROC

; 265  : {

$LN10:
	mov	DWORD PTR [rsp+24], r8d
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 266  : 	AuTextOut("read mem\n");

	lea	rcx, OFFSET FLAT:$SG8748
	call	AuTextOut

; 267  : 	switch (Width)

	mov	eax, DWORD PTR Width$[rsp]
	mov	DWORD PTR tv65[rsp], eax
	cmp	DWORD PTR tv65[rsp], 8
	je	SHORT $LN5@AcpiOsRead
	cmp	DWORD PTR tv65[rsp], 16
	je	SHORT $LN4@AcpiOsRead
	cmp	DWORD PTR tv65[rsp], 32			; 00000020H
	je	SHORT $LN3@AcpiOsRead
	cmp	DWORD PTR tv65[rsp], 64			; 00000040H
	je	SHORT $LN2@AcpiOsRead
	jmp	SHORT $LN1@AcpiOsRead
$LN5@AcpiOsRead:

; 268  : 	{
; 269  : 	case 8:
; 270  : 		*Value = *(UINT8*)Address;

	mov	rax, QWORD PTR Address$[rsp]
	movzx	eax, BYTE PTR [rax]
	mov	rcx, QWORD PTR Value$[rsp]
	mov	QWORD PTR [rcx], rax

; 271  : 		break;

	jmp	SHORT $LN6@AcpiOsRead
$LN4@AcpiOsRead:

; 272  : 	case 16:
; 273  : 		*Value = *(UINT16*)Address;

	mov	rax, QWORD PTR Address$[rsp]
	movzx	eax, WORD PTR [rax]
	mov	rcx, QWORD PTR Value$[rsp]
	mov	QWORD PTR [rcx], rax

; 274  : 		break;

	jmp	SHORT $LN6@AcpiOsRead
$LN3@AcpiOsRead:

; 275  : 	case 32:
; 276  : 		*Value = *(UINT32*)Address;

	mov	rax, QWORD PTR Address$[rsp]
	mov	eax, DWORD PTR [rax]
	mov	rcx, QWORD PTR Value$[rsp]
	mov	QWORD PTR [rcx], rax

; 277  : 		break;

	jmp	SHORT $LN6@AcpiOsRead
$LN2@AcpiOsRead:

; 278  : 	case 64:
; 279  : 		*Value = *(UINT64*)Address;

	mov	rax, QWORD PTR Value$[rsp]
	mov	rcx, QWORD PTR Address$[rsp]
	mov	rcx, QWORD PTR [rcx]
	mov	QWORD PTR [rax], rcx

; 280  : 		break;

	jmp	SHORT $LN6@AcpiOsRead
$LN1@AcpiOsRead:

; 281  : 	default:
; 282  : 		return AE_BAD_VALUE;

	mov	eax, 8196				; 00002004H
	jmp	SHORT $LN8@AcpiOsRead
$LN6@AcpiOsRead:

; 283  : 	}
; 284  : 	return AE_OK;

	xor	eax, eax
$LN8@AcpiOsRead:

; 285  : }

	add	rsp, 56					; 00000038H
	ret	0
AcpiOsReadMemory ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
tv65 = 32
Address$ = 64
Value$ = 72
Width$ = 80
AcpiOsWritePort PROC

; 339  : {

$LN7:
	mov	DWORD PTR [rsp+24], r8d
	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 340  : 	AuTextOut("Write port\n");

	lea	rcx, OFFSET FLAT:$SG8804
	call	AuTextOut

; 341  : 	switch (Width)

	mov	eax, DWORD PTR Width$[rsp]
	mov	DWORD PTR tv65[rsp], eax
	cmp	DWORD PTR tv65[rsp], 8
	je	SHORT $LN2@AcpiOsWrit
	cmp	DWORD PTR tv65[rsp], 16
	je	SHORT $LN2@AcpiOsWrit
	cmp	DWORD PTR tv65[rsp], 32			; 00000020H
	je	SHORT $LN2@AcpiOsWrit
	jmp	SHORT $LN1@AcpiOsWrit
$LN2@AcpiOsWrit:

; 342  : 	{
; 343  : 	case 8:
; 344  : 	case 16:
; 345  : 	case 32:
; 346  : 		x64_outportd(Address, Value);

	mov	edx, DWORD PTR Value$[rsp]
	movzx	ecx, WORD PTR Address$[rsp]
	call	x64_outportd

; 347  : 		break;

	jmp	SHORT $LN3@AcpiOsWrit
$LN1@AcpiOsWrit:

; 348  : 	default:
; 349  : 		return AE_BAD_VALUE;

	mov	eax, 8196				; 00002004H
	jmp	SHORT $LN5@AcpiOsWrit
$LN3@AcpiOsWrit:

; 350  : 	}
; 351  : 	return AE_OK;

	xor	eax, eax
$LN5@AcpiOsWrit:

; 352  : }

	add	rsp, 56					; 00000038H
	ret	0
AcpiOsWritePort ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
tv65 = 32
Address$ = 64
Value$ = 72
Width$ = 80
AcpiOsReadPort PROC

; 319  : {

$LN7:
	mov	DWORD PTR [rsp+24], r8d
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 320  : 	AuTextOut("read port\n");

	lea	rcx, OFFSET FLAT:$SG8790
	call	AuTextOut

; 321  : 	switch (Width)

	mov	eax, DWORD PTR Width$[rsp]
	mov	DWORD PTR tv65[rsp], eax
	cmp	DWORD PTR tv65[rsp], 8
	je	SHORT $LN2@AcpiOsRead
	cmp	DWORD PTR tv65[rsp], 16
	je	SHORT $LN2@AcpiOsRead
	cmp	DWORD PTR tv65[rsp], 32			; 00000020H
	je	SHORT $LN2@AcpiOsRead
	jmp	SHORT $LN1@AcpiOsRead
$LN2@AcpiOsRead:

; 322  : 	{
; 323  : 	case 8:
; 324  : 	case 16:
; 325  : 	case 32:
; 326  : 		*Value = x64_inportd(Address);

	movzx	ecx, WORD PTR Address$[rsp]
	call	x64_inportd
	mov	rcx, QWORD PTR Value$[rsp]
	mov	DWORD PTR [rcx], eax

; 327  : 		break;

	jmp	SHORT $LN3@AcpiOsRead
$LN1@AcpiOsRead:

; 328  : 	default:
; 329  : 		return AE_BAD_VALUE;

	mov	eax, 8196				; 00002004H
	jmp	SHORT $LN5@AcpiOsRead
$LN3@AcpiOsRead:

; 330  : 	}
; 331  : 	return AE_OK;

	xor	eax, eax
$LN5@AcpiOsRead:

; 332  : }

	add	rsp, 56					; 00000038H
	ret	0
AcpiOsReadPort ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
i$1 = 0
Micros$ = 32
AcpiOsStall PROC

; 126  : AU_EXTERN AU_FUNC void AcpiOsStall(UINT32 Micros) {

$LN6:
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 24

; 127  : 	for (int i = 0; i < Micros; i++)

	mov	DWORD PTR i$1[rsp], 0
	jmp	SHORT $LN3@AcpiOsStal
$LN2@AcpiOsStal:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN3@AcpiOsStal:
	mov	eax, DWORD PTR Micros$[rsp]
	cmp	DWORD PTR i$1[rsp], eax
	jae	SHORT $LN1@AcpiOsStal

; 128  : 		;

	jmp	SHORT $LN2@AcpiOsStal
$LN1@AcpiOsStal:

; 129  : }

	add	rsp, 24
	ret	0
AcpiOsStall ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
i$1 = 0
Millis$ = 32
AcpiOsSleep PROC

; 121  : AU_EXTERN AU_FUNC void AcpiOsSleep(UINT64 Millis) {

$LN6:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 24

; 122  : 	for (int i = 0; i < Millis; i++)

	mov	DWORD PTR i$1[rsp], 0
	jmp	SHORT $LN3@AcpiOsSlee
$LN2@AcpiOsSlee:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN3@AcpiOsSlee:
	movsxd	rax, DWORD PTR i$1[rsp]
	cmp	rax, QWORD PTR Millis$[rsp]
	jae	SHORT $LN1@AcpiOsSlee

; 123  : 		;

	jmp	SHORT $LN2@AcpiOsSlee
$LN1@AcpiOsSlee:

; 124  : }

	add	rsp, 24
	ret	0
AcpiOsSleep ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
AcpiOsWaitEventsComplete PROC

; 361  : }

	ret	0
AcpiOsWaitEventsComplete ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Type$ = 48
Function$ = 56
Context$ = 64
AcpiOsExecute PROC

; 115  : AU_EXTERN AU_FUNC ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context) {

$LN3:
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 40					; 00000028H

; 116  : 	Function(Context);

	mov	rcx, QWORD PTR Context$[rsp]
	call	QWORD PTR Function$[rsp]

; 117  : 	/* here we need scheduler to execute threads */
; 118  : 	return AE_OK;

	xor	eax, eax

; 119  : }

	add	rsp, 40					; 00000028H
	ret	0
AcpiOsExecute ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
AcpiOsGetThreadId PROC

; 112  : 	return (ACPI_THREAD_ID)-1;

	mov	rax, -1

; 113  : }

	ret	0
AcpiOsGetThreadId ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
InterruptNumber$ = 8
Handler$ = 16
AcpiOsRemoveInterruptHandler PROC

; 220  : {

	mov	QWORD PTR [rsp+16], rdx
	mov	DWORD PTR [rsp+8], ecx

; 221  : 	return AE_OK;

	xor	eax, eax

; 222  : }

	ret	0
AcpiOsRemoveInterruptHandler ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
InterruptLevel$ = 8
Handler$ = 16
Context$ = 24
AcpiOsInstallInterruptHandler PROC

; 215  : AU_EXTERN AU_FUNC ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptLevel, ACPI_OSD_HANDLER Handler, void *Context){

	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	DWORD PTR [rsp+8], ecx

; 216  : 	return AE_OK;

	xor	eax, eax

; 217  : }

	ret	0
AcpiOsInstallInterruptHandler ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
phys$ = 32
logicalAddress$ = 64
PhysicalAddress$ = 72
AcpiOsGetPhysicalAddress PROC

; 84   : AU_EXTERN AU_FUNC ACPI_STATUS AcpiOsGetPhysicalAddress(void* logicalAddress, ACPI_PHYSICAL_ADDRESS *PhysicalAddress) {

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 85   : 	void* phys = AuGetPhysicalAddress((uint64_t)logicalAddress);

	mov	rcx, QWORD PTR logicalAddress$[rsp]
	call	AuGetPhysicalAddress
	mov	QWORD PTR phys$[rsp], rax

; 86   : 	if (phys)

	cmp	QWORD PTR phys$[rsp], 0
	je	SHORT $LN2@AcpiOsGetP

; 87   : 		*PhysicalAddress = (ACPI_PHYSICAL_ADDRESS)phys;

	mov	rax, QWORD PTR PhysicalAddress$[rsp]
	mov	rcx, QWORD PTR phys$[rsp]
	mov	QWORD PTR [rax], rcx

; 88   : 	else

	jmp	SHORT $LN1@AcpiOsGetP
$LN2@AcpiOsGetP:

; 89   : 		*PhysicalAddress = (ACPI_PHYSICAL_ADDRESS)logicalAddress;

	mov	rax, QWORD PTR PhysicalAddress$[rsp]
	mov	rcx, QWORD PTR logicalAddress$[rsp]
	mov	QWORD PTR [rax], rcx
$LN1@AcpiOsGetP:

; 90   : 
; 91   : 	return AE_OK;

	xor	eax, eax

; 92   : }

	add	rsp, 56					; 00000038H
	ret	0
AcpiOsGetPhysicalAddress ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
where$ = 48
length$ = 56
AcpiOsUnmapMemory PROC

; 80   : AU_EXTERN AU_FUNC void AcpiOsUnmapMemory(void* where, ACPI_SIZE length) {

$LN3:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 81   : 	AuFreePages((uint64_t)where, true, length);

	mov	r8, QWORD PTR length$[rsp]
	mov	dl, 1
	mov	rcx, QWORD PTR where$[rsp]
	call	AuFreePages

; 82   : }

	add	rsp, 40					; 00000028H
	ret	0
AcpiOsUnmapMemory ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
i$1 = 32
align_off$ = 40
virt$ = 48
loc$ = 56
phys$ = 64
PhysicalAddress$ = 96
Length$ = 104
AcpiOsMapMemory PROC

; 70   : AU_EXTERN AU_FUNC void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length) {

$LN6:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 88					; 00000058H

; 71   : 	size_t align_off = PhysicalAddress & (PAGE_SIZE - 1);

	mov	rax, QWORD PTR PhysicalAddress$[rsp]
	and	rax, 4095				; 00000fffH
	mov	QWORD PTR align_off$[rsp], rax

; 72   : 	uint64_t* loc = AuGetFreePage(false, NULL);

	xor	edx, edx
	xor	ecx, ecx
	call	AuGetFreePage
	mov	QWORD PTR loc$[rsp], rax

; 73   : 	uint64_t virt = (uint64_t)loc;

	mov	rax, QWORD PTR loc$[rsp]
	mov	QWORD PTR virt$[rsp], rax

; 74   : 	uint64_t phys = PhysicalAddress - align_off;

	mov	rax, QWORD PTR align_off$[rsp]
	mov	rcx, QWORD PTR PhysicalAddress$[rsp]
	sub	rcx, rax
	mov	rax, rcx
	mov	QWORD PTR phys$[rsp], rax

; 75   : 	for (int i = 0; i < Length + align_off; i++)

	mov	DWORD PTR i$1[rsp], 0
	jmp	SHORT $LN3@AcpiOsMapM
$LN2@AcpiOsMapM:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN3@AcpiOsMapM:
	movsxd	rax, DWORD PTR i$1[rsp]
	mov	rcx, QWORD PTR align_off$[rsp]
	mov	rdx, QWORD PTR Length$[rsp]
	add	rdx, rcx
	mov	rcx, rdx
	cmp	rax, rcx
	jae	SHORT $LN1@AcpiOsMapM

; 76   : 		AuMapPage(phys + i * 4096, virt + i * 4096, 0);

	imul	eax, DWORD PTR i$1[rsp], 4096		; 00001000H
	cdqe
	mov	rcx, QWORD PTR virt$[rsp]
	add	rcx, rax
	mov	rax, rcx
	imul	ecx, DWORD PTR i$1[rsp], 4096		; 00001000H
	movsxd	rcx, ecx
	mov	rdx, QWORD PTR phys$[rsp]
	add	rdx, rcx
	mov	rcx, rdx
	xor	r8d, r8d
	mov	rdx, rax
	call	AuMapPage
	jmp	SHORT $LN2@AcpiOsMapM
$LN1@AcpiOsMapM:

; 77   : 	return (void*)(virt + align_off);

	mov	rax, QWORD PTR align_off$[rsp]
	mov	rcx, QWORD PTR virt$[rsp]
	add	rcx, rax
	mov	rax, rcx

; 78   : }

	add	rsp, 88					; 00000058H
	ret	0
AcpiOsMapMemory ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
memory$ = 48
AcpiOsFree PROC

; 98   : AU_EXTERN AU_FUNC void AcpiOsFree(void* memory) {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 99   : 	AuTextOut("Freeing up \n");

	lea	rcx, OFFSET FLAT:$SG8588
	call	AuTextOut

; 100  : 	return kfree(memory);

	mov	rcx, QWORD PTR memory$[rsp]
	call	kfree

; 101  : }

	add	rsp, 40					; 00000028H
	ret	0
AcpiOsFree ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Size$ = 48
AcpiOsAllocate PROC

; 94   : AU_EXTERN AU_FUNC void* AcpiOsAllocate(ACPI_SIZE Size) {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 95   : 	return kmalloc(Size);

	mov	ecx, DWORD PTR Size$[rsp]
	call	kmalloc

; 96   : }

	add	rsp, 40					; 00000028H
	ret	0
AcpiOsAllocate ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Handle$ = 8
AcpiOsReleaseMutex PROC

; 155  : {

	mov	QWORD PTR [rsp+8], rcx

; 156  : 	/*mutex_t* mut = (mutex_t*)Handle;
; 157  : 	mut->locked = 0;*/
; 158  : 	//AcpiOsSignalSemaphore(Handle, 1);
; 159  : }

	ret	0
AcpiOsReleaseMutex ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Handle$ = 8
Timeout$ = 16
AcpiOsAcquireMutex PROC

; 149  : {

	mov	WORD PTR [rsp+16], dx
	mov	QWORD PTR [rsp+8], rcx

; 150  : 	/*mutex_t* mut = (mutex_t*)Handle;
; 151  : 	mut->locked = 1;*/
; 152  : 	return AE_OK;//AcpiOsWaitSemaphore(Handle, 1, Timeout);

	xor	eax, eax

; 153  : }

	ret	0
AcpiOsAcquireMutex ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Handle$ = 48
AcpiOsDeleteMutex PROC

; 143  : AU_EXTERN AU_FUNC void AcpiOsDeleteMutex(ACPI_MUTEX Handle) {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 144  : 	kfree(Handle);

	mov	rcx, QWORD PTR Handle$[rsp]
	call	kfree

; 145  : }

	add	rsp, 40					; 00000028H
	ret	0
AcpiOsDeleteMutex ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
mut$ = 32
OutHandle$ = 64
AcpiOsCreateMutex PROC

; 135  : AU_EXTERN AU_FUNC ACPI_STATUS AcpiOsCreateMutex(ACPI_MUTEX *OutHandle) {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 136  : 	
; 137  : 	mutex_t *mut = (mutex_t*)kmalloc(sizeof(mutex_t));

	mov	ecx, 1
	call	kmalloc
	mov	QWORD PTR mut$[rsp], rax

; 138  : 	mut->locked = 0;

	mov	rax, QWORD PTR mut$[rsp]
	mov	BYTE PTR [rax], 0

; 139  : 	*OutHandle = mut;

	mov	rax, QWORD PTR OutHandle$[rsp]
	mov	rcx, QWORD PTR mut$[rsp]
	mov	QWORD PTR [rax], rcx

; 140  : 	return AE_OK;//AcpiOsCreateSemaphore(1, 1, OutHandle);

	xor	eax, eax

; 141  : }

	add	rsp, 56					; 00000038H
	ret	0
AcpiOsCreateMutex ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Handle$ = 8
Units$ = 16
AcpiOsSignalSemaphore PROC

; 177  : {

	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx

; 178  : 	return AE_OK;

	xor	eax, eax

; 179  : }

	ret	0
AcpiOsSignalSemaphore ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Handle$ = 8
Units$ = 16
Timeout$ = 24
AcpiOsWaitSemaphore PROC

; 172  : {

	mov	WORD PTR [rsp+24], r8w
	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx

; 173  : 	return AE_OK;

	xor	eax, eax

; 174  : }

	ret	0
AcpiOsWaitSemaphore ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Handle$ = 8
AcpiOsDeleteSemaphore PROC

; 167  : {

	mov	QWORD PTR [rsp+8], rcx

; 168  : 	return AE_OK;

	xor	eax, eax

; 169  : }

	ret	0
AcpiOsDeleteSemaphore ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
MaxUnits$ = 8
InitialUnits$ = 16
OutHandle$ = 24
AcpiOsCreateSemaphore PROC

; 162  : {

	mov	QWORD PTR [rsp+24], r8
	mov	DWORD PTR [rsp+16], edx
	mov	DWORD PTR [rsp+8], ecx

; 163  : 	return AE_OK;

	xor	eax, eax

; 164  : }

	ret	0
AcpiOsCreateSemaphore ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Handle$ = 8
Flags$ = 16
AcpiOsReleaseLock PROC

; 199  : {

	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx

; 200  : }

	ret	0
AcpiOsReleaseLock ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Handle$ = 8
AcpiOsAcquireLock PROC

; 194  : {

	mov	QWORD PTR [rsp+8], rcx

; 195  : 	return AE_OK;

	xor	eax, eax

; 196  : }

	ret	0
AcpiOsAcquireLock ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
Handle$ = 8
AcpiOsDeleteLock PROC

; 190  : {

	mov	QWORD PTR [rsp+8], rcx

; 191  : }

	ret	0
AcpiOsDeleteLock ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
mut$ = 32
OutHandle$ = 64
AcpiOsCreateLock PROC

; 182  : {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 183  : 	mutex_t *mut = (mutex_t*)kmalloc(sizeof(mutex_t));

	mov	ecx, 1
	call	kmalloc
	mov	QWORD PTR mut$[rsp], rax

; 184  : 	mut->locked = 0;

	mov	rax, QWORD PTR mut$[rsp]
	mov	BYTE PTR [rax], 0

; 185  : 	*OutHandle = mut;

	mov	rax, QWORD PTR OutHandle$[rsp]
	mov	rcx, QWORD PTR mut$[rsp]
	mov	QWORD PTR [rax], rcx

; 186  : 	return AE_OK;

	xor	eax, eax

; 187  : }

	add	rsp, 56					; 00000038H
	ret	0
AcpiOsCreateLock ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
ExistingTable$ = 8
NewAddress$ = 16
NewTableLength$ = 24
AcpiOsPhysicalTableOverride PROC

; 64   : 	UINT32 *NewTableLength) {

	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx

; 65   : 	*NewAddress = 0;

	mov	rax, QWORD PTR NewAddress$[rsp]
	mov	QWORD PTR [rax], 0

; 66   : 	*NewTableLength = 0;

	mov	rax, QWORD PTR NewTableLength$[rsp]
	mov	DWORD PTR [rax], 0

; 67   : 	return AE_OK;

	xor	eax, eax

; 68   : }

	ret	0
AcpiOsPhysicalTableOverride ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
ExistingTable$ = 8
NewTable$ = 16
AcpiOsTableOverride PROC

; 58   : AU_EXTERN AU_EXTERN AU_FUNC ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_TABLE_HEADER **NewTable) {

	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx

; 59   : 	*NewTable = 0;

	mov	rax, QWORD PTR NewTable$[rsp]
	mov	QWORD PTR [rax], 0

; 60   : 	return AE_OK;

	xor	eax, eax

; 61   : }

	ret	0
AcpiOsTableOverride ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
PredefinedObject$ = 8
NewValue$ = 16
AcpiOsPredefinedOverride PROC

; 53   : AU_EXTERN AU_FUNC ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES *PredefinedObject, ACPI_STRING *NewValue) {

	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx

; 54   : 	*NewValue = 0;

	mov	rax, QWORD PTR NewValue$[rsp]
	mov	QWORD PTR [rax], 0

; 55   : 	return AE_OK;

	xor	eax, eax

; 56   : }

	ret	0
AcpiOsPredefinedOverride ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
rsdp$ = 32
AcpiOsGetRootPointer PROC

; 48   : AU_EXTERN AU_FUNC ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer() {

$LN3:
	sub	rsp, 56					; 00000038H

; 49   : 	void* rsdp = AuACPIGetRSDP();

	call	?AuACPIGetRSDP@@YAPEAXXZ		; AuACPIGetRSDP
	mov	QWORD PTR rsdp$[rsp], rax

; 50   : 	return (ACPI_PHYSICAL_ADDRESS)rsdp;

	mov	rax, QWORD PTR rsdp$[rsp]

; 51   : }

	add	rsp, 56					; 00000038H
	ret	0
AcpiOsGetRootPointer ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
AcpiOsTerminate PROC

; 45   : 	return AE_OK;

	xor	eax, eax

; 46   : }

	ret	0
AcpiOsTerminate ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
AcpiOsInitialize PROC

; 41   : 	return AE_OK;

	xor	eax, eax

; 42   : }

	ret	0
AcpiOsInitialize ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
lock$ = 8
AcpiOsReleaseGlobalLock PROC

; 242  : {

	mov	QWORD PTR [rsp+8], rcx

; 243  : 	return AE_OK;

	xor	eax, eax

; 244  : }

	ret	0
AcpiOsReleaseGlobalLock ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\hal\acpiosl.cpp
_TEXT	SEGMENT
lock$ = 8
AcpiOsAcquireGlobalLock PROC

; 238  : {

	mov	QWORD PTR [rsp+8], rcx

; 239  : 	return AE_OK;

	xor	eax, eax

; 240  : }

	ret	0
AcpiOsAcquireGlobalLock ENDP
_TEXT	ENDS
END