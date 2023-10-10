; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?mice_@@3PEAU__VFS_NODE__@@EA			; mice_
PUBLIC	?kybrd_@@3PEAU__VFS_NODE__@@EA			; kybrd_
_BSS	SEGMENT
?mice_@@3PEAU__VFS_NODE__@@EA DQ 01H DUP (?)		; mice_
?kybrd_@@3PEAU__VFS_NODE__@@EA DQ 01H DUP (?)		; kybrd_
_BSS	ENDS
CONST	SEGMENT
$SG3460	DB	'/dev', 00H
	ORG $+3
$SG3468	DB	'mice', 00H
	ORG $+3
$SG3469	DB	'/', 00H
	ORG $+2
$SG3471	DB	'kybrd', 00H
	ORG $+2
$SG3472	DB	'/', 00H
CONST	ENDS
PUBLIC	?AuDevInputInitialise@@YAXXZ			; AuDevInputInitialise
PUBLIC	AuDevReadMice
PUBLIC	AuDevWriteMice
PUBLIC	?AuDevInputMiceWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ; AuDevInputMiceWrite
PUBLIC	?AuDevInputMiceRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ; AuDevInputMiceRead
PUBLIC	?AuDevMouseIoControl@@YAHPEAU__VFS_NODE__@@HPEAX@Z ; AuDevMouseIoControl
EXTRN	AuVFSFind:PROC
EXTRN	?AuCreatePipe@@YAPEAU__VFS_NODE__@@PEAD_K@Z:PROC ; AuCreatePipe
EXTRN	AuDevFSAddFile:PROC
EXTRN	strcpy:PROC
EXTRN	memset:PROC
EXTRN	memcpy:PROC
EXTRN	kmalloc:PROC
EXTRN	?AuPS2MouseSetPos@@YAXHH@Z:PROC			; AuPS2MouseSetPos
pdata	SEGMENT
$pdata$?AuDevInputInitialise@@YAXXZ DD imagerel $LN3
	DD	imagerel $LN3+311
	DD	imagerel $unwind$?AuDevInputInitialise@@YAXXZ
$pdata$AuDevReadMice DD imagerel $LN4
	DD	imagerel $LN4+77
	DD	imagerel $unwind$AuDevReadMice
$pdata$AuDevWriteMice DD imagerel $LN4
	DD	imagerel $LN4+53
	DD	imagerel $unwind$AuDevWriteMice
$pdata$?AuDevInputMiceWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD imagerel $LN5
	DD	imagerel $LN5+93
	DD	imagerel $unwind$?AuDevInputMiceWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z
$pdata$?AuDevInputMiceRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD imagerel $LN5
	DD	imagerel $LN5+111
	DD	imagerel $unwind$?AuDevInputMiceRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z
$pdata$?AuDevMouseIoControl@@YAHPEAU__VFS_NODE__@@HPEAX@Z DD imagerel $LN9
	DD	imagerel $LN9+105
	DD	imagerel $unwind$?AuDevMouseIoControl@@YAHPEAU__VFS_NODE__@@HPEAX@Z
pdata	ENDS
xdata	SEGMENT
$unwind$?AuDevInputInitialise@@YAXXZ DD 010401H
	DD	08204H
$unwind$AuDevReadMice DD 010901H
	DD	04209H
$unwind$AuDevWriteMice DD 010901H
	DD	04209H
$unwind$?AuDevInputMiceWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD 011801H
	DD	06218H
$unwind$?AuDevInputMiceRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD 011801H
	DD	06218H
$unwind$?AuDevMouseIoControl@@YAHPEAU__VFS_NODE__@@HPEAX@Z DD 011201H
	DD	06212H
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\dev\devinput.cpp
_TEXT	SEGMENT
tv67 = 32
ioctl$ = 40
file$ = 64
code$ = 72
arg$ = 80
?AuDevMouseIoControl@@YAHPEAU__VFS_NODE__@@HPEAX@Z PROC	; AuDevMouseIoControl

; 107  : int AuDevMouseIoControl(AuVFSNode* file, int code, void* arg) {

$LN9:
	mov	QWORD PTR [rsp+24], r8
	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 108  : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN6@AuDevMouse

; 109  : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN7@AuDevMouse
$LN6@AuDevMouse:

; 110  : 	AuFileIOControl *ioctl = (AuFileIOControl*)arg;

	mov	rax, QWORD PTR arg$[rsp]
	mov	QWORD PTR ioctl$[rsp], rax

; 111  : 	if (ioctl->syscall_magic != AURORA_SYSCALL_MAGIC)

	mov	rax, QWORD PTR ioctl$[rsp]
	cmp	DWORD PTR [rax], 86056964		; 05212004H
	je	SHORT $LN5@AuDevMouse

; 112  : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN7@AuDevMouse
$LN5@AuDevMouse:

; 113  : 
; 114  : 	switch (code)

	mov	eax, DWORD PTR code$[rsp]
	mov	DWORD PTR tv67[rsp], eax
	cmp	DWORD PTR tv67[rsp], 10
	je	SHORT $LN2@AuDevMouse
	jmp	SHORT $LN1@AuDevMouse
$LN2@AuDevMouse:

; 115  : 	{
; 116  : 	case MOUSE_IOCODE_SETPOS:
; 117  : 		AuPS2MouseSetPos(ioctl->uint_1, ioctl->uint_2);

	mov	rax, QWORD PTR ioctl$[rsp]
	mov	edx, DWORD PTR [rax+14]
	mov	rax, QWORD PTR ioctl$[rsp]
	mov	ecx, DWORD PTR [rax+10]
	call	?AuPS2MouseSetPos@@YAXHH@Z		; AuPS2MouseSetPos
$LN1@AuDevMouse:

; 118  : 		break;
; 119  : 	default:
; 120  : 		break;
; 121  : 	}
; 122  : 
; 123  : 	return 1;

	mov	eax, 1
$LN7@AuDevMouse:

; 124  : }

	add	rsp, 56					; 00000038H
	ret	0
?AuDevMouseIoControl@@YAHPEAU__VFS_NODE__@@HPEAX@Z ENDP	; AuDevMouseIoControl
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\dev\devinput.cpp
_TEXT	SEGMENT
mice_buf$ = 32
fs$ = 64
file$ = 72
buffer$ = 80
length$ = 88
?AuDevInputMiceRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z PROC ; AuDevInputMiceRead

; 90   : size_t AuDevInputMiceRead(AuVFSNode *fs, AuVFSNode *file, uint64_t* buffer, uint32_t length){

$LN5:
	mov	DWORD PTR [rsp+32], r9d
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 91   : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN2@AuDevInput

; 92   : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN3@AuDevInput
$LN2@AuDevInput:

; 93   : 	if (!buffer)

	cmp	QWORD PTR buffer$[rsp], 0
	jne	SHORT $LN1@AuDevInput

; 94   : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN3@AuDevInput
$LN1@AuDevInput:

; 95   : 	void* mice_buf = file->device;

	mov	rax, QWORD PTR file$[rsp]
	mov	rax, QWORD PTR [rax+64]
	mov	QWORD PTR mice_buf$[rsp], rax

; 96   : 	memcpy(buffer, mice_buf, sizeof(AuInputMessage));

	mov	r8d, 26
	mov	rdx, QWORD PTR mice_buf$[rsp]
	mov	rcx, QWORD PTR buffer$[rsp]
	call	memcpy

; 97   : 	memset(mice_buf, 0, sizeof(AuInputMessage));

	mov	r8d, 26
	xor	edx, edx
	mov	rcx, QWORD PTR mice_buf$[rsp]
	call	memset

; 98   : 	return (sizeof(AuInputMessage));

	mov	eax, 26
$LN3@AuDevInput:

; 99   : }

	add	rsp, 56					; 00000038H
	ret	0
?AuDevInputMiceRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ENDP ; AuDevInputMiceRead
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\dev\devinput.cpp
_TEXT	SEGMENT
mice_buf$ = 32
fs$ = 64
file$ = 72
buffer$ = 80
length$ = 88
?AuDevInputMiceWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z PROC ; AuDevInputMiceWrite

; 73   : size_t AuDevInputMiceWrite(AuVFSNode *fs, AuVFSNode *file, uint64_t* buffer, uint32_t length){

$LN5:
	mov	DWORD PTR [rsp+32], r9d
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 74   : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN2@AuDevInput

; 75   : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN3@AuDevInput
$LN2@AuDevInput:

; 76   : 	if (!buffer)

	cmp	QWORD PTR buffer$[rsp], 0
	jne	SHORT $LN1@AuDevInput

; 77   : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN3@AuDevInput
$LN1@AuDevInput:

; 78   : 	void* mice_buf = file->device;

	mov	rax, QWORD PTR file$[rsp]
	mov	rax, QWORD PTR [rax+64]
	mov	QWORD PTR mice_buf$[rsp], rax

; 79   : 	memcpy(mice_buf, buffer, sizeof(AuInputMessage));

	mov	r8d, 26
	mov	rdx, QWORD PTR buffer$[rsp]
	mov	rcx, QWORD PTR mice_buf$[rsp]
	call	memcpy

; 80   : 	return (sizeof(AuInputMessage));

	mov	eax, 26
$LN3@AuDevInput:

; 81   : }

	add	rsp, 56					; 00000038H
	ret	0
?AuDevInputMiceWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ENDP ; AuDevInputMiceWrite
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\dev\devinput.cpp
_TEXT	SEGMENT
outmsg$ = 48
AuDevWriteMice PROC

; 60   : void AuDevWriteMice(AuInputMessage* outmsg) {

$LN4:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 61   : 	if (!mice_)

	cmp	QWORD PTR ?mice_@@3PEAU__VFS_NODE__@@EA, 0 ; mice_
	jne	SHORT $LN1@AuDevWrite

; 62   : 		return;

	jmp	SHORT $LN2@AuDevWrite
$LN1@AuDevWrite:

; 63   : 	memcpy(mice_->device, outmsg, sizeof(AuInputMessage));

	mov	r8d, 26
	mov	rdx, QWORD PTR outmsg$[rsp]
	mov	rax, QWORD PTR ?mice_@@3PEAU__VFS_NODE__@@EA ; mice_
	mov	rcx, QWORD PTR [rax+64]
	call	memcpy
$LN2@AuDevWrite:

; 64   : }

	add	rsp, 40					; 00000028H
	ret	0
AuDevWriteMice ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\dev\devinput.cpp
_TEXT	SEGMENT
inputmsg$ = 48
AuDevReadMice PROC

; 49   : void AuDevReadMice(AuInputMessage* inputmsg) {

$LN4:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 50   : 	if (!mice_)

	cmp	QWORD PTR ?mice_@@3PEAU__VFS_NODE__@@EA, 0 ; mice_
	jne	SHORT $LN1@AuDevReadM

; 51   : 		return;

	jmp	SHORT $LN2@AuDevReadM
$LN1@AuDevReadM:

; 52   : 	memcpy(inputmsg, mice_->device, sizeof(AuInputMessage));

	mov	r8d, 26
	mov	rax, QWORD PTR ?mice_@@3PEAU__VFS_NODE__@@EA ; mice_
	mov	rdx, QWORD PTR [rax+64]
	mov	rcx, QWORD PTR inputmsg$[rsp]
	call	memcpy

; 53   : 	memset(mice_->device, 0, sizeof(AuInputMessage));

	mov	r8d, 26
	xor	edx, edx
	mov	rax, QWORD PTR ?mice_@@3PEAU__VFS_NODE__@@EA ; mice_
	mov	rcx, QWORD PTR [rax+64]
	call	memset
$LN2@AuDevReadM:

; 54   : }

	add	rsp, 40					; 00000028H
	ret	0
AuDevReadMice ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\dev\devinput.cpp
_TEXT	SEGMENT
node$ = 32
mice_input_buf$ = 40
devfs$ = 48
?AuDevInputInitialise@@YAXXZ PROC			; AuDevInputInitialise

; 130  : void AuDevInputInitialise() {

$LN3:
	sub	rsp, 72					; 00000048H

; 131  : 	AuVFSNode* devfs = AuVFSFind("/dev");

	lea	rcx, OFFSET FLAT:$SG3460
	call	AuVFSFind
	mov	QWORD PTR devfs$[rsp], rax

; 132  : 
; 133  : 	void* mice_input_buf = kmalloc(sizeof(AuInputMessage));

	mov	ecx, 26
	call	kmalloc
	mov	QWORD PTR mice_input_buf$[rsp], rax

; 134  : 	memset(mice_input_buf, 0, sizeof(AuInputMessage));

	mov	r8d, 26
	xor	edx, edx
	mov	rcx, QWORD PTR mice_input_buf$[rsp]
	call	memset

; 135  : 	/* avoiding using pipe for latency issue */
; 136  : 	AuVFSNode* node = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));

	mov	ecx, 160				; 000000a0H
	call	kmalloc
	mov	QWORD PTR node$[rsp], rax

; 137  : 	memset(node, 0, sizeof(AuVFSNode));

	mov	r8d, 160				; 000000a0H
	xor	edx, edx
	mov	rcx, QWORD PTR node$[rsp]
	call	memset

; 138  : 	strcpy(node->filename, "mice");

	mov	rax, QWORD PTR node$[rsp]
	lea	rdx, OFFSET FLAT:$SG3468
	mov	rcx, rax
	call	strcpy

; 139  : 	node->flags |= FS_FLAG_DEVICE;

	mov	rax, QWORD PTR node$[rsp]
	movzx	eax, WORD PTR [rax+61]
	or	eax, 8
	mov	rcx, QWORD PTR node$[rsp]
	mov	WORD PTR [rcx+61], ax

; 140  : 	node->device = mice_input_buf;

	mov	rax, QWORD PTR node$[rsp]
	mov	rcx, QWORD PTR mice_input_buf$[rsp]
	mov	QWORD PTR [rax+64], rcx

; 141  : 	node->read = AuDevInputMiceRead;

	mov	rax, QWORD PTR node$[rsp]
	lea	rcx, OFFSET FLAT:?AuDevInputMiceRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ; AuDevInputMiceRead
	mov	QWORD PTR [rax+80], rcx

; 142  : 	node->write = AuDevInputMiceWrite;

	mov	rax, QWORD PTR node$[rsp]
	lea	rcx, OFFSET FLAT:?AuDevInputMiceWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ; AuDevInputMiceWrite
	mov	QWORD PTR [rax+88], rcx

; 143  : 	node->open = 0;

	mov	rax, QWORD PTR node$[rsp]
	mov	QWORD PTR [rax+72], 0

; 144  : 	node->close = 0;

	mov	rax, QWORD PTR node$[rsp]
	mov	QWORD PTR [rax+128], 0

; 145  : 	node->iocontrol = AuDevMouseIoControl;

	mov	rax, QWORD PTR node$[rsp]
	lea	rcx, OFFSET FLAT:?AuDevMouseIoControl@@YAHPEAU__VFS_NODE__@@HPEAX@Z ; AuDevMouseIoControl
	mov	QWORD PTR [rax+152], rcx

; 146  : 	mice_ = node;

	mov	rax, QWORD PTR node$[rsp]
	mov	QWORD PTR ?mice_@@3PEAU__VFS_NODE__@@EA, rax ; mice_

; 147  : 	AuDevFSAddFile(devfs, "/", mice_);

	mov	r8, QWORD PTR ?mice_@@3PEAU__VFS_NODE__@@EA ; mice_
	lea	rdx, OFFSET FLAT:$SG3469
	mov	rcx, QWORD PTR devfs$[rsp]
	call	AuDevFSAddFile

; 148  : 	
; 149  : 	kybrd_ = AuCreatePipe("kybrd", sizeof(AuInputMessage)* NUM_KEYBOARD_PACKETS);

	mov	edx, 13312				; 00003400H
	lea	rcx, OFFSET FLAT:$SG3471
	call	?AuCreatePipe@@YAPEAU__VFS_NODE__@@PEAD_K@Z ; AuCreatePipe
	mov	QWORD PTR ?kybrd_@@3PEAU__VFS_NODE__@@EA, rax ; kybrd_

; 150  : 	AuDevFSAddFile(devfs, "/", kybrd_);

	mov	r8, QWORD PTR ?kybrd_@@3PEAU__VFS_NODE__@@EA ; kybrd_
	lea	rdx, OFFSET FLAT:$SG3472
	mov	rcx, QWORD PTR devfs$[rsp]
	call	AuDevFSAddFile

; 151  : }

	add	rsp, 72					; 00000048H
	ret	0
?AuDevInputInitialise@@YAXXZ ENDP			; AuDevInputInitialise
_TEXT	ENDS
END