; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?netadapters@@3PEAU_list_@@EA			; netadapters
_BSS	SEGMENT
?netadapters@@3PEAU_list_@@EA DQ 01H DUP (?)		; netadapters
_BSS	ENDS
PUBLIC	?AuInitialiseNet@@YAXXZ				; AuInitialiseNet
PUBLIC	AuAllocNetworkAdapter
PUBLIC	AuGetNetworkAdapterByType
EXTRN	initialize_list:PROC
EXTRN	list_add:PROC
EXTRN	list_get_at:PROC
EXTRN	kmalloc:PROC
EXTRN	memset:PROC
pdata	SEGMENT
$pdata$?AuInitialiseNet@@YAXXZ DD imagerel $LN3
	DD	imagerel $LN3+21
	DD	imagerel $unwind$?AuInitialiseNet@@YAXXZ
$pdata$AuAllocNetworkAdapter DD imagerel $LN3
	DD	imagerel $LN3+64
	DD	imagerel $unwind$AuAllocNetworkAdapter
$pdata$AuGetNetworkAdapterByType DD imagerel $LN7
	DD	imagerel $LN7+98
	DD	imagerel $unwind$AuGetNetworkAdapterByType
pdata	ENDS
xdata	SEGMENT
$unwind$?AuInitialiseNet@@YAXXZ DD 010401H
	DD	04204H
$unwind$AuAllocNetworkAdapter DD 010401H
	DD	06204H
$unwind$AuGetNetworkAdapterByType DD 010801H
	DD	06208H
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\net\aunet.cpp
_TEXT	SEGMENT
i$1 = 32
adapter$2 = 40
type$ = 64
AuGetNetworkAdapterByType PROC

; 58   : AuNetAdapter* AuGetNetworkAdapterByType(uint8_t type) {

$LN7:
	mov	BYTE PTR [rsp+8], cl
	sub	rsp, 56					; 00000038H

; 59   : 	for (int i = 0; i < netadapters->pointer; i++) {

	mov	DWORD PTR i$1[rsp], 0
	jmp	SHORT $LN4@AuGetNetwo
$LN3@AuGetNetwo:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN4@AuGetNetwo:
	mov	rax, QWORD PTR ?netadapters@@3PEAU_list_@@EA ; netadapters
	mov	eax, DWORD PTR [rax]
	cmp	DWORD PTR i$1[rsp], eax
	jae	SHORT $LN2@AuGetNetwo

; 60   : 		AuNetAdapter* adapter = (AuNetAdapter*)list_get_at(netadapters, i);

	mov	edx, DWORD PTR i$1[rsp]
	mov	rcx, QWORD PTR ?netadapters@@3PEAU_list_@@EA ; netadapters
	call	list_get_at
	mov	QWORD PTR adapter$2[rsp], rax

; 61   : 		if (adapter->type == type)

	mov	rax, QWORD PTR adapter$2[rsp]
	movzx	eax, BYTE PTR [rax+22]
	movzx	ecx, BYTE PTR type$[rsp]
	cmp	eax, ecx
	jne	SHORT $LN1@AuGetNetwo

; 62   : 			return adapter;

	mov	rax, QWORD PTR adapter$2[rsp]
	jmp	SHORT $LN5@AuGetNetwo
$LN1@AuGetNetwo:

; 63   : 	}

	jmp	SHORT $LN3@AuGetNetwo
$LN2@AuGetNetwo:

; 64   : 	return NULL;

	xor	eax, eax
$LN5@AuGetNetwo:

; 65   : }

	add	rsp, 56					; 00000038H
	ret	0
AuGetNetworkAdapterByType ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\net\aunet.cpp
_TEXT	SEGMENT
netadapt$ = 32
AuAllocNetworkAdapter PROC

; 47   : AuNetAdapter* AuAllocNetworkAdapter() {

$LN3:
	sub	rsp, 56					; 00000038H

; 48   : 	AuNetAdapter *netadapt = (AuNetAdapter*)kmalloc(sizeof(AuNetAdapter));

	mov	ecx, 23
	call	kmalloc
	mov	QWORD PTR netadapt$[rsp], rax

; 49   : 	memset(netadapt, 0, sizeof(AuNetAdapter));

	mov	r8d, 23
	xor	edx, edx
	mov	rcx, QWORD PTR netadapt$[rsp]
	call	memset

; 50   : 	list_add(netadapters, netadapt);

	mov	rdx, QWORD PTR netadapt$[rsp]
	mov	rcx, QWORD PTR ?netadapters@@3PEAU_list_@@EA ; netadapters
	call	list_add

; 51   : 	return netadapt;

	mov	rax, QWORD PTR netadapt$[rsp]

; 52   : }

	add	rsp, 56					; 00000038H
	ret	0
AuAllocNetworkAdapter ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\net\aunet.cpp
_TEXT	SEGMENT
?AuInitialiseNet@@YAXXZ PROC				; AuInitialiseNet

; 40   : void AuInitialiseNet() {

$LN3:
	sub	rsp, 40					; 00000028H

; 41   : 	netadapters = initialize_list();

	call	initialize_list
	mov	QWORD PTR ?netadapters@@3PEAU_list_@@EA, rax ; netadapters

; 42   : }

	add	rsp, 40					; 00000028H
	ret	0
?AuInitialiseNet@@YAXXZ ENDP				; AuInitialiseNet
_TEXT	ENDS
END
