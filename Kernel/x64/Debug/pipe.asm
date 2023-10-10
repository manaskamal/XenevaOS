; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?AuPipeUnread@@YA_KPEAU_pipe_@@@Z		; AuPipeUnread
PUBLIC	?AuCreatePipe@@YAPEAU__VFS_NODE__@@PEAD_K@Z	; AuCreatePipe
PUBLIC	?AuPipeGetAvailableBytes@@YA_KPEAU_pipe_@@@Z	; AuPipeGetAvailableBytes
PUBLIC	?AuPipeIncrementRead@@YAXPEAU_pipe_@@@Z		; AuPipeIncrementRead
PUBLIC	?AuPipeIncrementWrite@@YAXPEAU_pipe_@@@Z	; AuPipeIncrementWrite
PUBLIC	?AuPipeIncrementWriteAmount@@YAXPEAU_pipe_@@_K@Z ; AuPipeIncrementWriteAmount
PUBLIC	?AuPipeRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z	; AuPipeRead
PUBLIC	?AuPipeWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z	; AuPipeWrite
PUBLIC	?AuPipeOpen@@YAPEAU__VFS_NODE__@@PEAU1@PEAD@Z	; AuPipeOpen
PUBLIC	?AuPipeClose@@YAHPEAU__VFS_NODE__@@0@Z		; AuPipeClose
EXTRN	initialize_list:PROC
EXTRN	kmalloc:PROC
EXTRN	kfree:PROC
EXTRN	strcpy:PROC
EXTRN	memset:PROC
pdata	SEGMENT
$pdata$?AuCreatePipe@@YAPEAU__VFS_NODE__@@PEAD_K@Z DD imagerel $LN3
	DD	imagerel $LN3+290
	DD	imagerel $unwind$?AuCreatePipe@@YAPEAU__VFS_NODE__@@PEAD_K@Z
$pdata$?AuPipeRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD imagerel $LN9
	DD	imagerel $LN9+190
	DD	imagerel $unwind$?AuPipeRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z
$pdata$?AuPipeWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD imagerel $LN8
	DD	imagerel $LN8+189
	DD	imagerel $unwind$?AuPipeWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z
$pdata$?AuPipeOpen@@YAPEAU__VFS_NODE__@@PEAU1@PEAD@Z DD imagerel $LN3
	DD	imagerel $LN3+56
	DD	imagerel $unwind$?AuPipeOpen@@YAPEAU__VFS_NODE__@@PEAU1@PEAD@Z
$pdata$?AuPipeClose@@YAHPEAU__VFS_NODE__@@0@Z DD imagerel $LN4
	DD	imagerel $LN4+135
	DD	imagerel $unwind$?AuPipeClose@@YAHPEAU__VFS_NODE__@@0@Z
pdata	ENDS
xdata	SEGMENT
$unwind$?AuCreatePipe@@YAPEAU__VFS_NODE__@@PEAD_K@Z DD 010e01H
	DD	0620eH
$unwind$?AuPipeRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD 011801H
	DD	08218H
$unwind$?AuPipeWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD 011801H
	DD	08218H
$unwind$?AuPipeOpen@@YAPEAU__VFS_NODE__@@PEAU1@PEAD@Z DD 010e01H
	DD	0220eH
$unwind$?AuPipeClose@@YAHPEAU__VFS_NODE__@@0@Z DD 010e01H
	DD	0620eH
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\pipe.cpp
_TEXT	SEGMENT
pipe$ = 32
fs$ = 64
file$ = 72
?AuPipeClose@@YAHPEAU__VFS_NODE__@@0@Z PROC		; AuPipeClose

; 143  : int AuPipeClose(AuVFSNode* fs, AuVFSNode* file) {

$LN4:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 144  : 	AuPipe* pipe = (AuPipe*)fs->device;

	mov	rax, QWORD PTR fs$[rsp]
	mov	rax, QWORD PTR [rax+64]
	mov	QWORD PTR pipe$[rsp], rax

; 145  : 	pipe->refcount--;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rax, QWORD PTR [rax+32]
	dec	rax
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rcx+32], rax

; 146  : 	if (pipe->refcount == 0) {

	mov	rax, QWORD PTR pipe$[rsp]
	cmp	QWORD PTR [rax+32], 0
	jne	SHORT $LN1@AuPipeClos

; 147  : 		kfree(pipe->buffer);

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rax]
	call	kfree

; 148  : 		kfree(pipe->readers_wait_queue);

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rax+40]
	call	kfree

; 149  : 		kfree(pipe->writers_wait_queue);

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rax+48]
	call	kfree

; 150  : 		kfree(pipe);

	mov	rcx, QWORD PTR pipe$[rsp]
	call	kfree

; 151  : 		fs->device = NULL;

	mov	rax, QWORD PTR fs$[rsp]
	mov	QWORD PTR [rax+64], 0
$LN1@AuPipeClos:

; 152  : 	}
; 153  : 	return 1;

	mov	eax, 1

; 154  : }

	add	rsp, 56					; 00000038H
	ret	0
?AuPipeClose@@YAHPEAU__VFS_NODE__@@0@Z ENDP		; AuPipeClose
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\pipe.cpp
_TEXT	SEGMENT
pipe$ = 0
node$ = 32
path$ = 40
?AuPipeOpen@@YAPEAU__VFS_NODE__@@PEAU1@PEAD@Z PROC	; AuPipeOpen

; 132  : AuVFSNode* AuPipeOpen(AuVFSNode *node, char* path){

$LN3:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 24

; 133  : 	AuPipe* pipe = (AuPipe*)node->device;

	mov	rax, QWORD PTR node$[rsp]
	mov	rax, QWORD PTR [rax+64]
	mov	QWORD PTR pipe$[rsp], rax

; 134  : 	pipe->refcount++;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rax, QWORD PTR [rax+32]
	inc	rax
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rcx+32], rax

; 135  : 	return node;

	mov	rax, QWORD PTR node$[rsp]

; 136  : }

	add	rsp, 24
	ret	0
?AuPipeOpen@@YAPEAU__VFS_NODE__@@PEAU1@PEAD@Z ENDP	; AuPipeOpen
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\pipe.cpp
_TEXT	SEGMENT
written$ = 32
pipe$ = 40
aligned_buff$ = 48
fs$ = 80
file$ = 88
buffer$ = 96
length$ = 104
?AuPipeWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z PROC	; AuPipeWrite

; 114  : size_t AuPipeWrite(AuVFSNode *fs, AuVFSNode *file, uint64_t* buffer, uint32_t length) {

$LN8:
	mov	DWORD PTR [rsp+32], r9d
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 72					; 00000048H

; 115  : 	uint8_t* aligned_buff = (uint8_t*)buffer;

	mov	rax, QWORD PTR buffer$[rsp]
	mov	QWORD PTR aligned_buff$[rsp], rax

; 116  : 	AuPipe* pipe = (AuPipe*)fs->device;

	mov	rax, QWORD PTR fs$[rsp]
	mov	rax, QWORD PTR [rax+64]
	mov	QWORD PTR pipe$[rsp], rax

; 117  : 
; 118  : 	size_t written = 0;

	mov	QWORD PTR written$[rsp], 0
$LN5@AuPipeWrit:

; 119  : 	while (written < length) {

	mov	eax, DWORD PTR length$[rsp]
	cmp	QWORD PTR written$[rsp], rax
	jae	SHORT $LN4@AuPipeWrit

; 120  : 		if (AuPipeGetAvailableBytes(pipe) > length) {

	mov	rcx, QWORD PTR pipe$[rsp]
	call	?AuPipeGetAvailableBytes@@YA_KPEAU_pipe_@@@Z ; AuPipeGetAvailableBytes
	mov	ecx, DWORD PTR length$[rsp]
	cmp	rax, rcx
	jbe	SHORT $LN3@AuPipeWrit
$LN2@AuPipeWrit:

; 121  : 			while (AuPipeGetAvailableBytes(pipe) > 0 && written < length) {

	mov	rcx, QWORD PTR pipe$[rsp]
	call	?AuPipeGetAvailableBytes@@YA_KPEAU_pipe_@@@Z ; AuPipeGetAvailableBytes
	test	rax, rax
	jbe	SHORT $LN1@AuPipeWrit
	mov	eax, DWORD PTR length$[rsp]
	cmp	QWORD PTR written$[rsp], rax
	jae	SHORT $LN1@AuPipeWrit

; 122  : 				pipe->buffer[pipe->write_ptr] = aligned_buff[written];

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rax, QWORD PTR [rax+8]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx]
	mov	rdx, QWORD PTR written$[rsp]
	mov	r8, QWORD PTR aligned_buff$[rsp]
	add	r8, rdx
	mov	rdx, r8
	movzx	edx, BYTE PTR [rdx]
	mov	BYTE PTR [rcx+rax], dl

; 123  : 				AuPipeIncrementWrite(pipe);

	mov	rcx, QWORD PTR pipe$[rsp]
	call	?AuPipeIncrementWrite@@YAXPEAU_pipe_@@@Z ; AuPipeIncrementWrite

; 124  : 				written++;

	mov	rax, QWORD PTR written$[rsp]
	inc	rax
	mov	QWORD PTR written$[rsp], rax

; 125  : 			}

	jmp	SHORT $LN2@AuPipeWrit
$LN1@AuPipeWrit:
$LN3@AuPipeWrit:

; 126  : 		}
; 127  : 	}

	jmp	SHORT $LN5@AuPipeWrit
$LN4@AuPipeWrit:

; 128  : 	
; 129  : 	return written;

	mov	rax, QWORD PTR written$[rsp]

; 130  : }

	add	rsp, 72					; 00000048H
	ret	0
?AuPipeWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ENDP	; AuPipeWrite
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\pipe.cpp
_TEXT	SEGMENT
collected$ = 32
pipe$ = 40
aligned_buff$ = 48
fs$ = 80
file$ = 88
buffer$ = 96
length$ = 104
?AuPipeRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z PROC	; AuPipeRead

; 86   : size_t AuPipeRead(AuVFSNode *fs, AuVFSNode *file, uint64_t* buffer, uint32_t length) {

$LN9:
	mov	DWORD PTR [rsp+32], r9d
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 72					; 00000048H

; 87   : 	uint8_t* aligned_buff = (uint8_t*)buffer;

	mov	rax, QWORD PTR buffer$[rsp]
	mov	QWORD PTR aligned_buff$[rsp], rax

; 88   : 	AuPipe *pipe = (AuPipe*)fs->device;

	mov	rax, QWORD PTR fs$[rsp]
	mov	rax, QWORD PTR [rax+64]
	mov	QWORD PTR pipe$[rsp], rax

; 89   : 
; 90   : 	size_t collected = 0;

	mov	QWORD PTR collected$[rsp], 0
$LN6@AuPipeRead:

; 91   : 	while (collected == 0) {

	cmp	QWORD PTR collected$[rsp], 0
	jne	SHORT $LN5@AuPipeRead

; 92   : 		if (AuPipeUnread(pipe) >= length) {

	mov	rcx, QWORD PTR pipe$[rsp]
	call	?AuPipeUnread@@YA_KPEAU_pipe_@@@Z	; AuPipeUnread
	mov	ecx, DWORD PTR length$[rsp]
	cmp	rax, rcx
	jb	SHORT $LN4@AuPipeRead
$LN3@AuPipeRead:

; 93   : 			while (AuPipeUnread(pipe) > 0 && collected < length) {

	mov	rcx, QWORD PTR pipe$[rsp]
	call	?AuPipeUnread@@YA_KPEAU_pipe_@@@Z	; AuPipeUnread
	test	rax, rax
	jbe	SHORT $LN2@AuPipeRead
	mov	eax, DWORD PTR length$[rsp]
	cmp	QWORD PTR collected$[rsp], rax
	jae	SHORT $LN2@AuPipeRead

; 94   : 				aligned_buff[collected] = pipe->buffer[pipe->read_ptr];

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rax, QWORD PTR [rax+16]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx]
	mov	rdx, QWORD PTR collected$[rsp]
	mov	r8, QWORD PTR aligned_buff$[rsp]
	add	r8, rdx
	mov	rdx, r8
	movzx	eax, BYTE PTR [rcx+rax]
	mov	BYTE PTR [rdx], al

; 95   : 				AuPipeIncrementRead(pipe);

	mov	rcx, QWORD PTR pipe$[rsp]
	call	?AuPipeIncrementRead@@YAXPEAU_pipe_@@@Z	; AuPipeIncrementRead

; 96   : 				collected++;

	mov	rax, QWORD PTR collected$[rsp]
	inc	rax
	mov	QWORD PTR collected$[rsp], rax

; 97   : 			}

	jmp	SHORT $LN3@AuPipeRead
$LN2@AuPipeRead:

; 98   : 			
; 99   : 		}
; 100  : 		else 

	jmp	SHORT $LN1@AuPipeRead
$LN4@AuPipeRead:

; 101  : 			break;

	jmp	SHORT $LN5@AuPipeRead
$LN1@AuPipeRead:

; 102  : 	}

	jmp	SHORT $LN6@AuPipeRead
$LN5@AuPipeRead:

; 103  : 
; 104  : 	return collected;

	mov	rax, QWORD PTR collected$[rsp]

; 105  : }

	add	rsp, 72					; 00000048H
	ret	0
?AuPipeRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ENDP	; AuPipeRead
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\pipe.cpp
_TEXT	SEGMENT
pipe$ = 8
amount$ = 16
?AuPipeIncrementWriteAmount@@YAXPEAU_pipe_@@_K@Z PROC	; AuPipeIncrementWriteAmount

; 75   : void AuPipeIncrementWriteAmount(AuPipe* pipe, size_t amount) {

	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx

; 76   : 	pipe->write_ptr = (pipe->write_ptr + amount) & pipe->size;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rax, QWORD PTR [rax+8]
	add	rax, QWORD PTR amount$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	and	rax, QWORD PTR [rcx+24]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rcx+8], rax

; 77   : }

	ret	0
?AuPipeIncrementWriteAmount@@YAXPEAU_pipe_@@_K@Z ENDP	; AuPipeIncrementWriteAmount
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\pipe.cpp
_TEXT	SEGMENT
pipe$ = 8
?AuPipeIncrementWrite@@YAXPEAU_pipe_@@@Z PROC		; AuPipeIncrementWrite

; 63   : void AuPipeIncrementWrite(AuPipe* pipe) {

	mov	QWORD PTR [rsp+8], rcx

; 64   : 	pipe->write_ptr++;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rax, QWORD PTR [rax+8]
	inc	rax
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rcx+8], rax

; 65   : 	if (pipe->write_ptr == pipe->size)

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+24]
	cmp	QWORD PTR [rax+8], rcx
	jne	SHORT $LN1@AuPipeIncr

; 66   : 		pipe->write_ptr = 0;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rax+8], 0
$LN1@AuPipeIncr:

; 67   : }

	ret	0
?AuPipeIncrementWrite@@YAXPEAU_pipe_@@@Z ENDP		; AuPipeIncrementWrite
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\pipe.cpp
_TEXT	SEGMENT
pipe$ = 8
?AuPipeIncrementRead@@YAXPEAU_pipe_@@@Z PROC		; AuPipeIncrementRead

; 57   : void AuPipeIncrementRead(AuPipe* pipe) {

	mov	QWORD PTR [rsp+8], rcx

; 58   : 	pipe->read_ptr++;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rax, QWORD PTR [rax+16]
	inc	rax
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rcx+16], rax

; 59   : 	if (pipe->read_ptr == pipe->size)

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+24]
	cmp	QWORD PTR [rax+16], rcx
	jne	SHORT $LN1@AuPipeIncr

; 60   : 		pipe->read_ptr = 0;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rax+16], 0
$LN1@AuPipeIncr:

; 61   : }

	ret	0
?AuPipeIncrementRead@@YAXPEAU_pipe_@@@Z ENDP		; AuPipeIncrementRead
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\pipe.cpp
_TEXT	SEGMENT
pipe$ = 8
?AuPipeGetAvailableBytes@@YA_KPEAU_pipe_@@@Z PROC	; AuPipeGetAvailableBytes

; 46   : size_t AuPipeGetAvailableBytes(AuPipe *pipe) {

	mov	QWORD PTR [rsp+8], rcx

; 47   : 	if (pipe->read_ptr == pipe->write_ptr) 

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+8]
	cmp	QWORD PTR [rax+16], rcx
	jne	SHORT $LN3@AuPipeGetA

; 48   : 		return pipe->size - 1;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rax, QWORD PTR [rax+24]
	dec	rax
	jmp	SHORT $LN4@AuPipeGetA
$LN3@AuPipeGetA:

; 49   : 
; 50   : 	if (pipe->read_ptr > pipe->write_ptr)

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+8]
	cmp	QWORD PTR [rax+16], rcx
	jbe	SHORT $LN2@AuPipeGetA

; 51   : 		return pipe->read_ptr - pipe->write_ptr - 1;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+8]
	mov	rax, QWORD PTR [rax+16]
	sub	rax, rcx
	dec	rax
	jmp	SHORT $LN4@AuPipeGetA

; 52   : 	else

	jmp	SHORT $LN1@AuPipeGetA
$LN2@AuPipeGetA:

; 53   : 		return (pipe->size - pipe->write_ptr) + pipe->read_ptr - 1;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+8]
	mov	rax, QWORD PTR [rax+24]
	sub	rax, rcx
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+16]
	lea	rax, QWORD PTR [rax+rcx-1]
$LN1@AuPipeGetA:
$LN4@AuPipeGetA:

; 54   : }

	ret	0
?AuPipeGetAvailableBytes@@YA_KPEAU_pipe_@@@Z ENDP	; AuPipeGetAvailableBytes
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\pipe.cpp
_TEXT	SEGMENT
node$ = 32
pipe$ = 40
name$ = 64
sz$ = 72
?AuCreatePipe@@YAPEAU__VFS_NODE__@@PEAD_K@Z PROC	; AuCreatePipe

; 161  : AuVFSNode* AuCreatePipe(char* name, size_t sz) {

$LN3:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 162  : 	AuVFSNode* node = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));

	mov	ecx, 160				; 000000a0H
	call	kmalloc
	mov	QWORD PTR node$[rsp], rax

; 163  : 	AuPipe *pipe = (AuPipe*)kmalloc(sizeof(AuPipe));

	mov	ecx, 56					; 00000038H
	call	kmalloc
	mov	QWORD PTR pipe$[rsp], rax

; 164  : 	memset(node, 0, sizeof(AuVFSNode));

	mov	r8d, 160				; 000000a0H
	xor	edx, edx
	mov	rcx, QWORD PTR node$[rsp]
	call	memset

; 165  : 	memset(pipe, 0, sizeof(AuPipe));

	mov	r8d, 56					; 00000038H
	xor	edx, edx
	mov	rcx, QWORD PTR pipe$[rsp]
	call	memset

; 166  : 
; 167  : 	pipe->buffer = (uint8_t*)kmalloc(sz);

	mov	ecx, DWORD PTR sz$[rsp]
	call	kmalloc
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rcx], rax

; 168  : 	pipe->readers_wait_queue = initialize_list();

	call	initialize_list
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rcx+40], rax

; 169  : 	pipe->writers_wait_queue = initialize_list();

	call	initialize_list
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rcx+48], rax

; 170  : 	pipe->size = sz;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR sz$[rsp]
	mov	QWORD PTR [rax+24], rcx

; 171  : 
; 172  : 	strcpy(node->filename, name);

	mov	rax, QWORD PTR node$[rsp]
	mov	rdx, QWORD PTR name$[rsp]
	mov	rcx, rax
	call	strcpy

; 173  : 	node->flags = FS_FLAG_DEVICE | FS_FLAG_PIPE;

	mov	eax, 136				; 00000088H
	mov	rcx, QWORD PTR node$[rsp]
	mov	WORD PTR [rcx+61], ax

; 174  : 	node->size = sz;

	mov	rax, QWORD PTR node$[rsp]
	mov	ecx, DWORD PTR sz$[rsp]
	mov	DWORD PTR [rax+32], ecx

; 175  : 	node->device = pipe;

	mov	rax, QWORD PTR node$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	QWORD PTR [rax+64], rcx

; 176  : 	node->read = AuPipeRead;

	mov	rax, QWORD PTR node$[rsp]
	lea	rcx, OFFSET FLAT:?AuPipeRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ; AuPipeRead
	mov	QWORD PTR [rax+80], rcx

; 177  : 	node->write = AuPipeWrite;

	mov	rax, QWORD PTR node$[rsp]
	lea	rcx, OFFSET FLAT:?AuPipeWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ; AuPipeWrite
	mov	QWORD PTR [rax+88], rcx

; 178  : 	node->open = AuPipeOpen;

	mov	rax, QWORD PTR node$[rsp]
	lea	rcx, OFFSET FLAT:?AuPipeOpen@@YAPEAU__VFS_NODE__@@PEAU1@PEAD@Z ; AuPipeOpen
	mov	QWORD PTR [rax+72], rcx

; 179  : 	node->close = AuPipeClose;

	mov	rax, QWORD PTR node$[rsp]
	lea	rcx, OFFSET FLAT:?AuPipeClose@@YAHPEAU__VFS_NODE__@@0@Z ; AuPipeClose
	mov	QWORD PTR [rax+128], rcx

; 180  : 	node->iocontrol = NULL;

	mov	rax, QWORD PTR node$[rsp]
	mov	QWORD PTR [rax+152], 0

; 181  : 	
; 182  : 	return node;

	mov	rax, QWORD PTR node$[rsp]

; 183  : }

	add	rsp, 56					; 00000038H
	ret	0
?AuCreatePipe@@YAPEAU__VFS_NODE__@@PEAD_K@Z ENDP	; AuCreatePipe
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\pipe.cpp
_TEXT	SEGMENT
pipe$ = 8
?AuPipeUnread@@YA_KPEAU_pipe_@@@Z PROC			; AuPipeUnread

; 37   : size_t AuPipeUnread(AuPipe* pipe) {

	mov	QWORD PTR [rsp+8], rcx

; 38   : 	if (pipe->read_ptr == pipe->write_ptr)

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+8]
	cmp	QWORD PTR [rax+16], rcx
	jne	SHORT $LN3@AuPipeUnre

; 39   : 		return 0; //0 bytes difference

	xor	eax, eax
	jmp	SHORT $LN4@AuPipeUnre
$LN3@AuPipeUnre:

; 40   : 	if (pipe->read_ptr > pipe->write_ptr)

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+8]
	cmp	QWORD PTR [rax+16], rcx
	jbe	SHORT $LN2@AuPipeUnre

; 41   : 		return (pipe->size - pipe->read_ptr) + pipe->write_ptr;

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+16]
	mov	rax, QWORD PTR [rax+24]
	sub	rax, rcx
	mov	rcx, QWORD PTR pipe$[rsp]
	add	rax, QWORD PTR [rcx+8]
	jmp	SHORT $LN4@AuPipeUnre

; 42   : 	else

	jmp	SHORT $LN1@AuPipeUnre
$LN2@AuPipeUnre:

; 43   : 		return (pipe->write_ptr - pipe->read_ptr);

	mov	rax, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR pipe$[rsp]
	mov	rcx, QWORD PTR [rcx+16]
	mov	rax, QWORD PTR [rax+8]
	sub	rax, rcx
$LN1@AuPipeUnre:
$LN4@AuPipeUnre:

; 44   : }

	ret	0
?AuPipeUnread@@YA_KPEAU_pipe_@@@Z ENDP			; AuPipeUnread
_TEXT	ENDS
END