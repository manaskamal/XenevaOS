; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?AuAdvancePointer@@YAXPEAU_circ_buf_@@@Z	; AuAdvancePointer
PUBLIC	?AuRetreatPointer@@YAXPEAU_circ_buf_@@@Z	; AuRetreatPointer
PUBLIC	?AuCircBufReset@@YAXPEAU_circ_buf_@@@Z		; AuCircBufReset
PUBLIC	?AuCircBufInitialise@@YAPEAU_circ_buf_@@PEAE_K@Z ; AuCircBufInitialise
PUBLIC	?AuCircBufFree@@YAXPEAU_circ_buf_@@@Z		; AuCircBufFree
PUBLIC	?AuCircBufSize@@YA_KPEAU_circ_buf_@@@Z		; AuCircBufSize
PUBLIC	?AuCircBufCapacity@@YA_KPEAU_circ_buf_@@@Z	; AuCircBufCapacity
PUBLIC	?AuCircBufPutData@@YAXPEAU_circ_buf_@@E@Z	; AuCircBufPutData
PUBLIC	?AuCircBufPut@@YAHPEAU_circ_buf_@@E@Z		; AuCircBufPut
PUBLIC	?AuCircBufGet@@YAHPEAU_circ_buf_@@PEAE@Z	; AuCircBufGet
PUBLIC	?CircBufEmpty@@YA_NPEAU_circ_buf_@@@Z		; CircBufEmpty
PUBLIC	?CircBufFull@@YA_NPEAU_circ_buf_@@@Z		; CircBufFull
EXTRN	kmalloc:PROC
EXTRN	kfree:PROC
pdata	SEGMENT
$pdata$?AuAdvancePointer@@YAXPEAU_circ_buf_@@@Z DD imagerel $LN6
	DD	imagerel $LN6+145
	DD	imagerel $unwind$?AuAdvancePointer@@YAXPEAU_circ_buf_@@@Z
$pdata$?AuCircBufInitialise@@YAPEAU_circ_buf_@@PEAE_K@Z DD imagerel $LN3
	DD	imagerel $LN3+76
	DD	imagerel $unwind$?AuCircBufInitialise@@YAPEAU_circ_buf_@@PEAE_K@Z
$pdata$?AuCircBufFree@@YAXPEAU_circ_buf_@@@Z DD imagerel $LN3
	DD	imagerel $LN3+24
	DD	imagerel $unwind$?AuCircBufFree@@YAXPEAU_circ_buf_@@@Z
$pdata$?AuCircBufSize@@YA_KPEAU_circ_buf_@@@Z DD imagerel $LN6
	DD	imagerel $LN6+122
	DD	imagerel $unwind$?AuCircBufSize@@YA_KPEAU_circ_buf_@@@Z
$pdata$?AuCircBufPutData@@YAXPEAU_circ_buf_@@E@Z DD imagerel $LN3
	DD	imagerel $LN3+53
	DD	imagerel $unwind$?AuCircBufPutData@@YAXPEAU_circ_buf_@@E@Z
$pdata$?AuCircBufPut@@YAHPEAU_circ_buf_@@E@Z DD imagerel $LN4
	DD	imagerel $LN4+90
	DD	imagerel $unwind$?AuCircBufPut@@YAHPEAU_circ_buf_@@E@Z
$pdata$?AuCircBufGet@@YAHPEAU_circ_buf_@@PEAE@Z DD imagerel $LN4
	DD	imagerel $LN4+94
	DD	imagerel $unwind$?AuCircBufGet@@YAHPEAU_circ_buf_@@PEAE@Z
$pdata$?CircBufEmpty@@YA_NPEAU_circ_buf_@@@Z DD imagerel $LN5
	DD	imagerel $LN5+67
	DD	imagerel $unwind$?CircBufEmpty@@YA_NPEAU_circ_buf_@@@Z
pdata	ENDS
xdata	SEGMENT
$unwind$?AuAdvancePointer@@YAXPEAU_circ_buf_@@@Z DD 010901H
	DD	02209H
$unwind$?AuCircBufInitialise@@YAPEAU_circ_buf_@@PEAE_K@Z DD 010e01H
	DD	0620eH
$unwind$?AuCircBufFree@@YAXPEAU_circ_buf_@@@Z DD 010901H
	DD	04209H
$unwind$?AuCircBufSize@@YA_KPEAU_circ_buf_@@@Z DD 010901H
	DD	02209H
$unwind$?AuCircBufPutData@@YAXPEAU_circ_buf_@@E@Z DD 010d01H
	DD	0420dH
$unwind$?AuCircBufPut@@YAHPEAU_circ_buf_@@E@Z DD 010d01H
	DD	0620dH
$unwind$?AuCircBufGet@@YAHPEAU_circ_buf_@@PEAE@Z DD 010e01H
	DD	0620eH
$unwind$?CircBufEmpty@@YA_NPEAU_circ_buf_@@@Z DD 010901H
	DD	02209H
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
cbuf$ = 8
?CircBufFull@@YA_NPEAU_circ_buf_@@@Z PROC		; CircBufFull

; 177  : {

	mov	QWORD PTR [rsp+8], rcx

; 178  : 	return cbuf->full;

	mov	rax, QWORD PTR cbuf$[rsp]
	movzx	eax, BYTE PTR [rax+32]

; 179  : }

	ret	0
?CircBufFull@@YA_NPEAU_circ_buf_@@@Z ENDP		; CircBufFull
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
tv70 = 0
cbuf$ = 32
?CircBufEmpty@@YA_NPEAU_circ_buf_@@@Z PROC		; CircBufEmpty

; 166  : {

$LN5:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 24

; 167  : 	return (!cbuf->full && (cbuf->head == cbuf->tail));

	mov	rax, QWORD PTR cbuf$[rsp]
	movzx	eax, BYTE PTR [rax+32]
	test	eax, eax
	jne	SHORT $LN3@CircBufEmp
	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR [rcx+16]
	cmp	QWORD PTR [rax+8], rcx
	jne	SHORT $LN3@CircBufEmp
	mov	DWORD PTR tv70[rsp], 1
	jmp	SHORT $LN4@CircBufEmp
$LN3@CircBufEmp:
	mov	DWORD PTR tv70[rsp], 0
$LN4@CircBufEmp:
	movzx	eax, BYTE PTR tv70[rsp]

; 168  : }

	add	rsp, 24
	ret	0
?CircBufEmpty@@YA_NPEAU_circ_buf_@@@Z ENDP		; CircBufEmpty
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
r$ = 32
cbuf$ = 64
data$ = 72
?AuCircBufGet@@YAHPEAU_circ_buf_@@PEAE@Z PROC		; AuCircBufGet

; 148  : {

$LN4:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 149  : 	int r = -1;

	mov	DWORD PTR r$[rsp], -1

; 150  : 
; 151  : 	if (!CircBufEmpty(cbuf)){

	mov	rcx, QWORD PTR cbuf$[rsp]
	call	?CircBufEmpty@@YA_NPEAU_circ_buf_@@@Z	; CircBufEmpty
	movzx	eax, al
	test	eax, eax
	jne	SHORT $LN1@AuCircBufG

; 152  : 		*data = cbuf->buffer[cbuf->tail];

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rax, QWORD PTR [rax+16]
	mov	rcx, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR [rcx]
	mov	rdx, QWORD PTR data$[rsp]
	movzx	eax, BYTE PTR [rcx+rax]
	mov	BYTE PTR [rdx], al

; 153  : 		AuRetreatPointer(cbuf);

	mov	rcx, QWORD PTR cbuf$[rsp]
	call	?AuRetreatPointer@@YAXPEAU_circ_buf_@@@Z ; AuRetreatPointer

; 154  : 		r = 0;

	mov	DWORD PTR r$[rsp], 0
$LN1@AuCircBufG:

; 155  : 	}
; 156  : 	return r;

	mov	eax, DWORD PTR r$[rsp]

; 157  : }

	add	rsp, 56					; 00000038H
	ret	0
?AuCircBufGet@@YAHPEAU_circ_buf_@@PEAE@Z ENDP		; AuCircBufGet
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
r$ = 32
cbuf$ = 64
data$ = 72
?AuCircBufPut@@YAHPEAU_circ_buf_@@E@Z PROC		; AuCircBufPut

; 129  : int AuCircBufPut(CircBuffer* cbuf, uint8_t data) {

$LN4:
	mov	BYTE PTR [rsp+16], dl
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 130  : 	int r = -1;

	mov	DWORD PTR r$[rsp], -1

; 131  : 	if (!CircBufFull(cbuf)) {

	mov	rcx, QWORD PTR cbuf$[rsp]
	call	?CircBufFull@@YA_NPEAU_circ_buf_@@@Z	; CircBufFull
	movzx	eax, al
	test	eax, eax
	jne	SHORT $LN1@AuCircBufP

; 132  : 		cbuf->buffer[cbuf->head] = data;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rax, QWORD PTR [rax+8]
	mov	rcx, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR [rcx]
	movzx	edx, BYTE PTR data$[rsp]
	mov	BYTE PTR [rcx+rax], dl

; 133  : 		AuAdvancePointer(cbuf);

	mov	rcx, QWORD PTR cbuf$[rsp]
	call	?AuAdvancePointer@@YAXPEAU_circ_buf_@@@Z ; AuAdvancePointer

; 134  : 		r = 0;

	mov	DWORD PTR r$[rsp], 0
$LN1@AuCircBufP:

; 135  : 	}
; 136  : 
; 137  : 	return r;

	mov	eax, DWORD PTR r$[rsp]

; 138  : }

	add	rsp, 56					; 00000038H
	ret	0
?AuCircBufPut@@YAHPEAU_circ_buf_@@E@Z ENDP		; AuCircBufPut
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
cbuf$ = 48
data$ = 56
?AuCircBufPutData@@YAXPEAU_circ_buf_@@E@Z PROC		; AuCircBufPutData

; 119  : void AuCircBufPutData(CircBuffer* cbuf, uint8_t data) {

$LN3:
	mov	BYTE PTR [rsp+16], dl
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 120  : 	cbuf->buffer[cbuf->head] = data;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rax, QWORD PTR [rax+8]
	mov	rcx, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR [rcx]
	movzx	edx, BYTE PTR data$[rsp]
	mov	BYTE PTR [rcx+rax], dl

; 121  : 	AuAdvancePointer(cbuf);

	mov	rcx, QWORD PTR cbuf$[rsp]
	call	?AuAdvancePointer@@YAXPEAU_circ_buf_@@@Z ; AuAdvancePointer

; 122  : }

	add	rsp, 40					; 00000028H
	ret	0
?AuCircBufPutData@@YAXPEAU_circ_buf_@@E@Z ENDP		; AuCircBufPutData
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
cbuf$ = 8
?AuCircBufCapacity@@YA_KPEAU_circ_buf_@@@Z PROC		; AuCircBufCapacity

; 110  : size_t AuCircBufCapacity(CircBuffer *cbuf) {

	mov	QWORD PTR [rsp+8], rcx

; 111  : 	return cbuf->max;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rax, QWORD PTR [rax+24]

; 112  : }

	ret	0
?AuCircBufCapacity@@YA_KPEAU_circ_buf_@@@Z ENDP		; AuCircBufCapacity
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
sz$ = 0
cbuf$ = 32
?AuCircBufSize@@YA_KPEAU_circ_buf_@@@Z PROC		; AuCircBufSize

; 94   : size_t AuCircBufSize(CircBuffer *cbuf) {

$LN6:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 24

; 95   : 	size_t sz = cbuf->max;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rax, QWORD PTR [rax+24]
	mov	QWORD PTR sz$[rsp], rax

; 96   : 
; 97   : 	if (!cbuf->full) {

	mov	rax, QWORD PTR cbuf$[rsp]
	movzx	eax, BYTE PTR [rax+32]
	test	eax, eax
	jne	SHORT $LN3@AuCircBufS

; 98   : 		if (cbuf->head >= cbuf->tail)

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR [rcx+16]
	cmp	QWORD PTR [rax+8], rcx
	jb	SHORT $LN2@AuCircBufS

; 99   : 			sz = (cbuf->head - cbuf->tail);

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR [rcx+16]
	mov	rax, QWORD PTR [rax+8]
	sub	rax, rcx
	mov	QWORD PTR sz$[rsp], rax

; 100  : 		else

	jmp	SHORT $LN1@AuCircBufS
$LN2@AuCircBufS:

; 101  : 			sz = (cbuf->max + cbuf->head - cbuf->tail);

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rax, QWORD PTR [rax+24]
	mov	rcx, QWORD PTR cbuf$[rsp]
	add	rax, QWORD PTR [rcx+8]
	mov	rcx, QWORD PTR cbuf$[rsp]
	sub	rax, QWORD PTR [rcx+16]
	mov	QWORD PTR sz$[rsp], rax
$LN1@AuCircBufS:
$LN3@AuCircBufS:

; 102  : 	}
; 103  : 	return sz;

	mov	rax, QWORD PTR sz$[rsp]

; 104  : }

	add	rsp, 24
	ret	0
?AuCircBufSize@@YA_KPEAU_circ_buf_@@@Z ENDP		; AuCircBufSize
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
cbuf$ = 48
?AuCircBufFree@@YAXPEAU_circ_buf_@@@Z PROC		; AuCircBufFree

; 84   : void AuCircBufFree(CircBuffer *cbuf) {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 85   : 	kfree(cbuf);

	mov	rcx, QWORD PTR cbuf$[rsp]
	call	kfree

; 86   : }

	add	rsp, 40					; 00000028H
	ret	0
?AuCircBufFree@@YAXPEAU_circ_buf_@@@Z ENDP		; AuCircBufFree
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
cbuf$ = 32
buffer$ = 64
sz$ = 72
?AuCircBufInitialise@@YAPEAU_circ_buf_@@PEAE_K@Z PROC	; AuCircBufInitialise

; 71   : CircBuffer* AuCircBufInitialise(uint8_t* buffer, size_t sz) {

$LN3:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 72   : 	CircBuffer *cbuf = (CircBuffer*)kmalloc(sizeof(CircBuffer));

	mov	ecx, 40					; 00000028H
	call	kmalloc
	mov	QWORD PTR cbuf$[rsp], rax

; 73   : 	cbuf->buffer = buffer;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR buffer$[rsp]
	mov	QWORD PTR [rax], rcx

; 74   : 	cbuf->max = sz;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR sz$[rsp]
	mov	QWORD PTR [rax+24], rcx

; 75   : 	AuCircBufReset(cbuf);

	mov	rcx, QWORD PTR cbuf$[rsp]
	call	?AuCircBufReset@@YAXPEAU_circ_buf_@@@Z	; AuCircBufReset

; 76   : 	return cbuf;

	mov	rax, QWORD PTR cbuf$[rsp]

; 77   : }

	add	rsp, 56					; 00000038H
	ret	0
?AuCircBufInitialise@@YAPEAU_circ_buf_@@PEAE_K@Z ENDP	; AuCircBufInitialise
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
cbuf$ = 8
?AuCircBufReset@@YAXPEAU_circ_buf_@@@Z PROC		; AuCircBufReset

; 60   : void AuCircBufReset(CircBuffer *cbuf) {

	mov	QWORD PTR [rsp+8], rcx

; 61   : 	cbuf->head = 0;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	QWORD PTR [rax+8], 0

; 62   : 	cbuf->tail = 0;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	QWORD PTR [rax+16], 0

; 63   : 	cbuf->full = false;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	BYTE PTR [rax+32], 0

; 64   : }

	ret	0
?AuCircBufReset@@YAXPEAU_circ_buf_@@@Z ENDP		; AuCircBufReset
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
cbuf$ = 8
?AuRetreatPointer@@YAXPEAU_circ_buf_@@@Z PROC		; AuRetreatPointer

; 51   : void AuRetreatPointer(CircBuffer *cbuf) {

	mov	QWORD PTR [rsp+8], rcx

; 52   : 	cbuf->full = false;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	BYTE PTR [rax+32], 0

; 53   : 	cbuf->tail = (cbuf->tail + 1) % cbuf->max;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rax, QWORD PTR [rax+16]
	inc	rax
	xor	edx, edx
	mov	rcx, QWORD PTR cbuf$[rsp]
	div	QWORD PTR [rcx+24]
	mov	rax, rdx
	mov	rcx, QWORD PTR cbuf$[rsp]
	mov	QWORD PTR [rcx+16], rax

; 54   : }

	ret	0
?AuRetreatPointer@@YAXPEAU_circ_buf_@@@Z ENDP		; AuRetreatPointer
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\circbuf.cpp
_TEXT	SEGMENT
tv81 = 0
cbuf$ = 32
?AuAdvancePointer@@YAXPEAU_circ_buf_@@@Z PROC		; AuAdvancePointer

; 38   : void AuAdvancePointer(CircBuffer *cbuf) {

$LN6:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 24

; 39   : 	if (cbuf->full)

	mov	rax, QWORD PTR cbuf$[rsp]
	movzx	eax, BYTE PTR [rax+32]
	test	eax, eax
	je	SHORT $LN1@AuAdvanceP

; 40   : 		cbuf->tail = (cbuf->tail + 1) % cbuf->max;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rax, QWORD PTR [rax+16]
	inc	rax
	xor	edx, edx
	mov	rcx, QWORD PTR cbuf$[rsp]
	div	QWORD PTR [rcx+24]
	mov	rax, rdx
	mov	rcx, QWORD PTR cbuf$[rsp]
	mov	QWORD PTR [rcx+16], rax
$LN1@AuAdvanceP:

; 41   : 
; 42   : 	cbuf->head = (cbuf->head + 1) % cbuf->max;

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rax, QWORD PTR [rax+8]
	inc	rax
	xor	edx, edx
	mov	rcx, QWORD PTR cbuf$[rsp]
	div	QWORD PTR [rcx+24]
	mov	rax, rdx
	mov	rcx, QWORD PTR cbuf$[rsp]
	mov	QWORD PTR [rcx+8], rax

; 43   : 	cbuf->full = (cbuf->head == cbuf->tail);

	mov	rax, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR cbuf$[rsp]
	mov	rcx, QWORD PTR [rcx+16]
	cmp	QWORD PTR [rax+8], rcx
	jne	SHORT $LN4@AuAdvanceP
	mov	DWORD PTR tv81[rsp], 1
	jmp	SHORT $LN5@AuAdvanceP
$LN4@AuAdvanceP:
	mov	DWORD PTR tv81[rsp], 0
$LN5@AuAdvanceP:
	mov	rax, QWORD PTR cbuf$[rsp]
	movzx	ecx, BYTE PTR tv81[rsp]
	mov	BYTE PTR [rax+32], cl

; 44   : }

	add	rsp, 24
	ret	0
?AuAdvancePointer@@YAXPEAU_circ_buf_@@@Z ENDP		; AuAdvancePointer
_TEXT	ENDS
END