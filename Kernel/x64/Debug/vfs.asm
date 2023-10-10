; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?__RootContainer@@3PEAU__VFS_Container__@@EA	; __RootContainer
PUBLIC	?__RootFS@@3PEAU__VFS_NODE__@@EA		; __RootFS
PUBLIC	?_vfs_debug_on@@3_NA				; _vfs_debug_on
_BSS	SEGMENT
?__RootContainer@@3PEAU__VFS_Container__@@EA DQ 01H DUP (?) ; __RootContainer
?__RootFS@@3PEAU__VFS_NODE__@@EA DQ 01H DUP (?)		; __RootFS
?_vfs_debug_on@@3_NA DB 01H DUP (?)			; _vfs_debug_on
_BSS	ENDS
PUBLIC	?AuVFSInitialise@@YAXXZ				; AuVFSInitialise
PUBLIC	AuVFSOpen
PUBLIC	AuVFSNodeIOControl
PUBLIC	AuVFSAddFileSystem
PUBLIC	AuVFSRegisterRoot
PUBLIC	AuVFSFind
PUBLIC	AuVFSNodeRead
PUBLIC	AuVFSNodeReadBlock
PUBLIC	AuVFSCreateDir
PUBLIC	AuVFSCreateFile
PUBLIC	?AuVFSRemoveFile@@YAHPEAU__VFS_NODE__@@0@Z	; AuVFSRemoveFile
PUBLIC	?AuVFSRemoveDir@@YAHPEAU__VFS_NODE__@@0@Z	; AuVFSRemoveDir
PUBLIC	AuVFSNodeWrite
PUBLIC	AuVFSNodeClose
PUBLIC	AuVFSGetBlockFor
PUBLIC	AuVFSRemoveFileSystem
EXTRN	initialize_list:PROC
EXTRN	list_add:PROC
EXTRN	list_remove:PROC
EXTRN	list_get_at:PROC
EXTRN	kmalloc:PROC
EXTRN	kfree:PROC
EXTRN	?AuDeviceFsInitialize@@YAXXZ:PROC		; AuDeviceFsInitialize
EXTRN	strcmp:PROC
EXTRN	strchr:PROC
pdata	SEGMENT
$pdata$?AuVFSInitialise@@YAXXZ DD imagerel $LN3
	DD	imagerel $LN3+72
	DD	imagerel $unwind$?AuVFSInitialise@@YAXXZ
$pdata$AuVFSOpen DD imagerel $LN16
	DD	imagerel $LN16+381
	DD	imagerel $unwind$AuVFSOpen
$pdata$AuVFSNodeIOControl DD imagerel $LN4
	DD	imagerel $LN4+75
	DD	imagerel $unwind$AuVFSNodeIOControl
$pdata$AuVFSAddFileSystem DD imagerel $LN7
	DD	imagerel $LN7+112
	DD	imagerel $unwind$AuVFSAddFileSystem
$pdata$AuVFSFind DD imagerel $LN14
	DD	imagerel $LN14+304
	DD	imagerel $unwind$AuVFSFind
$pdata$AuVFSNodeRead DD imagerel $LN5
	DD	imagerel $LN5+86
	DD	imagerel $unwind$AuVFSNodeRead
$pdata$AuVFSNodeReadBlock DD imagerel $LN5
	DD	imagerel $LN5+99
	DD	imagerel $unwind$AuVFSNodeReadBlock
$pdata$AuVFSCreateDir DD imagerel $LN5
	DD	imagerel $LN5+80
	DD	imagerel $unwind$AuVFSCreateDir
$pdata$AuVFSCreateFile DD imagerel $LN5
	DD	imagerel $LN5+80
	DD	imagerel $unwind$AuVFSCreateFile
$pdata$?AuVFSRemoveFile@@YAHPEAU__VFS_NODE__@@0@Z DD imagerel $LN9
	DD	imagerel $LN9+153
	DD	imagerel $unwind$?AuVFSRemoveFile@@YAHPEAU__VFS_NODE__@@0@Z
$pdata$?AuVFSRemoveDir@@YAHPEAU__VFS_NODE__@@0@Z DD imagerel $LN8
	DD	imagerel $LN8+134
	DD	imagerel $unwind$?AuVFSRemoveDir@@YAHPEAU__VFS_NODE__@@0@Z
$pdata$AuVFSNodeWrite DD imagerel $LN5
	DD	imagerel $LN5+79
	DD	imagerel $unwind$AuVFSNodeWrite
$pdata$AuVFSNodeClose DD imagerel $LN4
	DD	imagerel $LN4+55
	DD	imagerel $unwind$AuVFSNodeClose
$pdata$AuVFSGetBlockFor DD imagerel $LN5
	DD	imagerel $LN5+82
	DD	imagerel $unwind$AuVFSGetBlockFor
$pdata$AuVFSRemoveFileSystem DD imagerel $LN9
	DD	imagerel $LN9+176
	DD	imagerel $unwind$AuVFSRemoveFileSystem
pdata	ENDS
xdata	SEGMENT
$unwind$?AuVFSInitialise@@YAXXZ DD 010401H
	DD	06204H
$unwind$AuVFSOpen DD 010901H
	DD	0c209H
$unwind$AuVFSNodeIOControl DD 011201H
	DD	06212H
$unwind$AuVFSAddFileSystem DD 010901H
	DD	06209H
$unwind$AuVFSFind DD 010901H
	DD	0a209H
$unwind$AuVFSNodeRead DD 011801H
	DD	04218H
$unwind$AuVFSNodeReadBlock DD 011301H
	DD	06213H
$unwind$AuVFSCreateDir DD 010e01H
	DD	0620eH
$unwind$AuVFSCreateFile DD 010e01H
	DD	0620eH
$unwind$?AuVFSRemoveFile@@YAHPEAU__VFS_NODE__@@0@Z DD 010e01H
	DD	0620eH
$unwind$?AuVFSRemoveDir@@YAHPEAU__VFS_NODE__@@0@Z DD 010e01H
	DD	0420eH
$unwind$AuVFSNodeWrite DD 011801H
	DD	04218H
$unwind$AuVFSNodeClose DD 010e01H
	DD	0420eH
$unwind$AuVFSGetBlockFor DD 011301H
	DD	04213H
$unwind$AuVFSRemoveFileSystem DD 010901H
	DD	06209H
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
i$1 = 32
index$ = 36
_node$2 = 40
node$ = 64
AuVFSRemoveFileSystem PROC

; 308  : AU_EXTERN AU_EXPORT int AuVFSRemoveFileSystem(AuVFSNode* node) {

$LN9:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 309  : 	int index = 0;

	mov	DWORD PTR index$[rsp], 0

; 310  : 	for (int i = 0; i < __RootContainer->childs->pointer; i++) {

	mov	DWORD PTR i$1[rsp], 0
	jmp	SHORT $LN6@AuVFSRemov
$LN5@AuVFSRemov:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN6@AuVFSRemov:
	mov	rax, QWORD PTR ?__RootContainer@@3PEAU__VFS_Container__@@EA ; __RootContainer
	mov	rax, QWORD PTR [rax]
	mov	eax, DWORD PTR [rax]
	cmp	DWORD PTR i$1[rsp], eax
	jae	SHORT $LN4@AuVFSRemov

; 311  : 		AuVFSNode *_node = (AuVFSNode*)list_get_at(__RootContainer->childs, i);

	mov	edx, DWORD PTR i$1[rsp]
	mov	rax, QWORD PTR ?__RootContainer@@3PEAU__VFS_Container__@@EA ; __RootContainer
	mov	rcx, QWORD PTR [rax]
	call	list_get_at
	mov	QWORD PTR _node$2[rsp], rax

; 312  : 		if (_node == node){

	mov	rax, QWORD PTR node$[rsp]
	cmp	QWORD PTR _node$2[rsp], rax
	jne	SHORT $LN3@AuVFSRemov

; 313  : 			index = i;

	mov	eax, DWORD PTR i$1[rsp]
	mov	DWORD PTR index$[rsp], eax

; 314  : 			break;

	jmp	SHORT $LN4@AuVFSRemov
$LN3@AuVFSRemov:

; 315  : 		}
; 316  : 	}

	jmp	SHORT $LN5@AuVFSRemov
$LN4@AuVFSRemov:

; 317  : 	list_remove(__RootContainer->childs, index);

	mov	edx, DWORD PTR index$[rsp]
	mov	rax, QWORD PTR ?__RootContainer@@3PEAU__VFS_Container__@@EA ; __RootContainer
	mov	rcx, QWORD PTR [rax]
	call	list_remove

; 318  : 	if (node->close)

	mov	rax, QWORD PTR node$[rsp]
	cmp	QWORD PTR [rax+128], 0
	je	SHORT $LN2@AuVFSRemov

; 319  : 		return node->close(node, NULL);

	xor	edx, edx
	mov	rcx, QWORD PTR node$[rsp]
	mov	rax, QWORD PTR node$[rsp]
	call	QWORD PTR [rax+128]
	jmp	SHORT $LN7@AuVFSRemov

; 320  : 	else {

	jmp	SHORT $LN1@AuVFSRemov
$LN2@AuVFSRemov:

; 321  : 		kfree(node);

	mov	rcx, QWORD PTR node$[rsp]
	call	kfree

; 322  : 		return 0;

	xor	eax, eax
$LN1@AuVFSRemov:
$LN7@AuVFSRemov:

; 323  : 	}
; 324  : 
; 325  : }

	add	rsp, 56					; 00000038H
	ret	0
AuVFSRemoveFileSystem ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
node$ = 48
file$ = 56
offset$ = 64
AuVFSGetBlockFor PROC

; 334  : AU_EXTERN AU_EXPORT size_t AuVFSGetBlockFor(AuVFSNode* node, AuVFSNode* file, uint64_t offset) {

$LN5:
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 335  : 	if (node){

	cmp	QWORD PTR node$[rsp], 0
	je	SHORT $LN2@AuVFSGetBl

; 336  : 		if (node->get_blockfor)

	mov	rax, QWORD PTR node$[rsp]
	cmp	QWORD PTR [rax+144], 0
	je	SHORT $LN1@AuVFSGetBl

; 337  : 			return node->get_blockfor(node, file, offset);

	mov	r8, QWORD PTR offset$[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR node$[rsp]
	mov	rax, QWORD PTR node$[rsp]
	call	QWORD PTR [rax+144]
	jmp	SHORT $LN3@AuVFSGetBl
$LN1@AuVFSGetBl:
$LN2@AuVFSGetBl:

; 338  : 	}
; 339  : 	return -1;

	mov	rax, -1
$LN3@AuVFSGetBl:

; 340  : }

	add	rsp, 40					; 00000028H
	ret	0
AuVFSGetBlockFor ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
node$ = 48
file$ = 56
AuVFSNodeClose PROC

; 299  : AU_EXTERN AU_EXPORT void AuVFSNodeClose(AuVFSNode* node, AuVFSNode* file) {

$LN4:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 300  : 	if (node->close)

	mov	rax, QWORD PTR node$[rsp]
	cmp	QWORD PTR [rax+128], 0
	je	SHORT $LN1@AuVFSNodeC

; 301  : 		node->close(node, file);

	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR node$[rsp]
	mov	rax, QWORD PTR node$[rsp]
	call	QWORD PTR [rax+128]
$LN1@AuVFSNodeC:

; 302  : }

	add	rsp, 40					; 00000028H
	ret	0
AuVFSNodeClose ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
node$ = 48
file$ = 56
buffer$ = 64
length$ = 72
AuVFSNodeWrite PROC

; 207  : AU_EXTERN AU_EXPORT void AuVFSNodeWrite(AuVFSNode* node, AuVFSNode * file, uint64_t *buffer, uint32_t length) {

$LN5:
	mov	DWORD PTR [rsp+32], r9d
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 208  : 	if (!node)

	cmp	QWORD PTR node$[rsp], 0
	jne	SHORT $LN2@AuVFSNodeW

; 209  : 		return;

	jmp	SHORT $LN3@AuVFSNodeW
$LN2@AuVFSNodeW:

; 210  : 	if (node->write)

	mov	rax, QWORD PTR node$[rsp]
	cmp	QWORD PTR [rax+88], 0
	je	SHORT $LN1@AuVFSNodeW

; 211  : 		node->write(node, file, buffer, length);

	mov	r9d, DWORD PTR length$[rsp]
	mov	r8, QWORD PTR buffer$[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR node$[rsp]
	mov	rax, QWORD PTR node$[rsp]
	call	QWORD PTR [rax+88]
$LN1@AuVFSNodeW:
$LN3@AuVFSNodeW:

; 212  : }

	add	rsp, 40					; 00000028H
	ret	0
AuVFSNodeWrite ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
fsys$ = 48
file$ = 56
?AuVFSRemoveDir@@YAHPEAU__VFS_NODE__@@0@Z PROC		; AuVFSRemoveDir

; 282  : int AuVFSRemoveDir(AuVFSNode* fsys, AuVFSNode* file) {

$LN8:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 283  : 	if (!fsys)

	cmp	QWORD PTR fsys$[rsp], 0
	jne	SHORT $LN5@AuVFSRemov

; 284  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN6@AuVFSRemov
$LN5@AuVFSRemov:

; 285  : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN4@AuVFSRemov

; 286  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN6@AuVFSRemov
$LN4@AuVFSRemov:

; 287  : 	if ((file->flags & FS_FLAG_DEVICE) || (file->flags & FS_FLAG_FILE_SYSTEM))

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 8
	test	eax, eax
	jne	SHORT $LN2@AuVFSRemov
	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 64					; 00000040H
	test	eax, eax
	je	SHORT $LN3@AuVFSRemov
$LN2@AuVFSRemov:

; 288  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN6@AuVFSRemov
$LN3@AuVFSRemov:

; 289  : 	if (fsys->remove_dir && (file->flags & FS_FLAG_DIRECTORY))

	mov	rax, QWORD PTR fsys$[rsp]
	cmp	QWORD PTR [rax+112], 0
	je	SHORT $LN1@AuVFSRemov
	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 2
	test	eax, eax
	je	SHORT $LN1@AuVFSRemov

; 290  : 		return fsys->remove_dir(fsys, file);

	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	mov	rax, QWORD PTR fsys$[rsp]
	call	QWORD PTR [rax+112]
$LN1@AuVFSRemov:
$LN6@AuVFSRemov:

; 291  : }

	add	rsp, 40					; 00000028H
	ret	0
?AuVFSRemoveDir@@YAHPEAU__VFS_NODE__@@0@Z ENDP		; AuVFSRemoveDir
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
ret$ = 32
fsys$ = 64
file$ = 72
?AuVFSRemoveFile@@YAHPEAU__VFS_NODE__@@0@Z PROC		; AuVFSRemoveFile

; 262  : int AuVFSRemoveFile(AuVFSNode* fsys, AuVFSNode* file) {

$LN9:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 263  : 	if (!fsys)

	cmp	QWORD PTR fsys$[rsp], 0
	jne	SHORT $LN6@AuVFSRemov

; 264  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN7@AuVFSRemov
$LN6@AuVFSRemov:

; 265  : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN5@AuVFSRemov

; 266  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN7@AuVFSRemov
$LN5@AuVFSRemov:

; 267  : 	if (!(file->flags & FS_FLAG_GENERAL))

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 4
	test	eax, eax
	jne	SHORT $LN4@AuVFSRemov

; 268  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN7@AuVFSRemov
$LN4@AuVFSRemov:

; 269  : 	if ((file->flags & FS_FLAG_DEVICE) || (file->flags & FS_FLAG_FILE_SYSTEM))

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 8
	test	eax, eax
	jne	SHORT $LN2@AuVFSRemov
	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 64					; 00000040H
	test	eax, eax
	je	SHORT $LN3@AuVFSRemov
$LN2@AuVFSRemov:

; 270  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN7@AuVFSRemov
$LN3@AuVFSRemov:

; 271  : 	int ret = -1;

	mov	DWORD PTR ret$[rsp], -1

; 272  : 	if (fsys->remove_file) 

	mov	rax, QWORD PTR fsys$[rsp]
	cmp	QWORD PTR [rax+120], 0
	je	SHORT $LN1@AuVFSRemov

; 273  : 		ret = fsys->remove_file(fsys, file);

	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	mov	rax, QWORD PTR fsys$[rsp]
	call	QWORD PTR [rax+120]
	mov	DWORD PTR ret$[rsp], eax
$LN1@AuVFSRemov:
$LN7@AuVFSRemov:

; 274  : }

	add	rsp, 56					; 00000038H
	ret	0
?AuVFSRemoveFile@@YAHPEAU__VFS_NODE__@@0@Z ENDP		; AuVFSRemoveFile
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
ret$ = 32
fsys$ = 64
filename$ = 72
AuVFSCreateFile PROC

; 248  : AU_EXTERN AU_EXPORT AuVFSNode* AuVFSCreateFile(AuVFSNode* fsys, char* filename){

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 249  : 	if (!fsys)

	cmp	QWORD PTR fsys$[rsp], 0
	jne	SHORT $LN2@AuVFSCreat

; 250  : 		return NULL;

	xor	eax, eax
	jmp	SHORT $LN3@AuVFSCreat
$LN2@AuVFSCreat:

; 251  : 	AuVFSNode* ret = NULL;

	mov	QWORD PTR ret$[rsp], 0

; 252  : 	if (fsys->create_file)

	mov	rax, QWORD PTR fsys$[rsp]
	cmp	QWORD PTR [rax+104], 0
	je	SHORT $LN1@AuVFSCreat

; 253  : 		ret = fsys->create_file(fsys, filename);

	mov	rdx, QWORD PTR filename$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	mov	rax, QWORD PTR fsys$[rsp]
	call	QWORD PTR [rax+104]
	mov	QWORD PTR ret$[rsp], rax
$LN1@AuVFSCreat:

; 254  : 	return ret;

	mov	rax, QWORD PTR ret$[rsp]
$LN3@AuVFSCreat:

; 255  : }

	add	rsp, 56					; 00000038H
	ret	0
AuVFSCreateFile ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
ret$ = 32
fsys$ = 64
dirname$ = 72
AuVFSCreateDir PROC

; 234  : AU_EXTERN AU_EXPORT AuVFSNode* AuVFSCreateDir(AuVFSNode* fsys, char* dirname) {

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 235  : 	if (!fsys)

	cmp	QWORD PTR fsys$[rsp], 0
	jne	SHORT $LN2@AuVFSCreat

; 236  : 		return NULL;

	xor	eax, eax
	jmp	SHORT $LN3@AuVFSCreat
$LN2@AuVFSCreat:

; 237  : 	AuVFSNode* ret = NULL;

	mov	QWORD PTR ret$[rsp], 0

; 238  : 	if (fsys->create_dir)

	mov	rax, QWORD PTR fsys$[rsp]
	cmp	QWORD PTR [rax+96], 0
	je	SHORT $LN1@AuVFSCreat

; 239  : 		ret = fsys->create_dir(fsys, dirname);

	mov	rdx, QWORD PTR dirname$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	mov	rax, QWORD PTR fsys$[rsp]
	call	QWORD PTR [rax+96]
	mov	QWORD PTR ret$[rsp], rax
$LN1@AuVFSCreat:

; 240  : 	return ret;

	mov	rax, QWORD PTR ret$[rsp]
$LN3@AuVFSCreat:

; 241  : }

	add	rsp, 56					; 00000038H
	ret	0
AuVFSCreateDir ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
read_bytes$ = 32
node$ = 64
file$ = 72
buffer$ = 80
AuVFSNodeReadBlock PROC

; 220  : AU_EXTERN AU_EXPORT size_t AuVFSNodeReadBlock(AuVFSNode* node, AuVFSNode* file, uint64_t *buffer) {

$LN5:
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 221  : 	size_t read_bytes = 0;

	mov	QWORD PTR read_bytes$[rsp], 0

; 222  : 	if (!node)

	cmp	QWORD PTR node$[rsp], 0
	jne	SHORT $LN2@AuVFSNodeR

; 223  : 		return read_bytes;

	mov	rax, QWORD PTR read_bytes$[rsp]
	jmp	SHORT $LN3@AuVFSNodeR
$LN2@AuVFSNodeR:

; 224  : 	if (node->read_block)

	mov	rax, QWORD PTR node$[rsp]
	cmp	QWORD PTR [rax+136], 0
	je	SHORT $LN1@AuVFSNodeR

; 225  : 		read_bytes = node->read_block(node, file, buffer);

	mov	r8, QWORD PTR buffer$[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR node$[rsp]
	mov	rax, QWORD PTR node$[rsp]
	call	QWORD PTR [rax+136]
	mov	QWORD PTR read_bytes$[rsp], rax
$LN1@AuVFSNodeR:

; 226  : 	return read_bytes;

	mov	rax, QWORD PTR read_bytes$[rsp]
$LN3@AuVFSNodeR:

; 227  : }

	add	rsp, 56					; 00000038H
	ret	0
AuVFSNodeReadBlock ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
node$ = 48
file$ = 56
buffer$ = 64
length$ = 72
AuVFSNodeRead PROC

; 192  : AU_EXTERN AU_EXPORT size_t AuVFSNodeRead(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t length) {

$LN5:
	mov	DWORD PTR [rsp+32], r9d
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 193  : 	if (node) {

	cmp	QWORD PTR node$[rsp], 0
	je	SHORT $LN2@AuVFSNodeR

; 194  : 		if (node->read)

	mov	rax, QWORD PTR node$[rsp]
	cmp	QWORD PTR [rax+80], 0
	je	SHORT $LN1@AuVFSNodeR

; 195  : 			return node->read(node, file, buffer, length);

	mov	r9d, DWORD PTR length$[rsp]
	mov	r8, QWORD PTR buffer$[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR node$[rsp]
	mov	rax, QWORD PTR node$[rsp]
	call	QWORD PTR [rax+80]
	jmp	SHORT $LN3@AuVFSNodeR
$LN1@AuVFSNodeR:
$LN2@AuVFSNodeR:

; 196  : 	}
; 197  : 	return -1;

	mov	rax, -1
$LN3@AuVFSNodeR:

; 198  : }

	add	rsp, 40					; 00000028H
	ret	0
AuVFSNodeRead ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
i$ = 32
j$1 = 36
next$ = 40
Returnable$ = 48
node$2 = 56
pathname$ = 64
path$ = 96
AuVFSFind PROC

; 65   : AuVFSNode* AuVFSFind(char* path) {

$LN14:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 88					; 00000058H

; 66   : 
; 67   : 	AuVFSNode* Returnable = NULL;

	mov	QWORD PTR Returnable$[rsp], 0

; 68   : 	/* first of all search all file system 
; 69   : 	 * skipping '/' of the path 
; 70   : 	 */
; 71   : 	char* next = strchr(path, '/');

	mov	edx, 47					; 0000002fH
	mov	rcx, QWORD PTR path$[rsp]
	call	strchr
	mov	QWORD PTR next$[rsp], rax

; 72   : 	if (next)

	cmp	QWORD PTR next$[rsp], 0
	je	SHORT $LN11@AuVFSFind

; 73   : 		next++;

	mov	rax, QWORD PTR next$[rsp]
	inc	rax
	mov	QWORD PTR next$[rsp], rax
$LN11@AuVFSFind:

; 74   : 
; 75   : 	char pathname[16];
; 76   : 	int i;
; 77   : 	for (i = 0; i < 16; i++) {

	mov	DWORD PTR i$[rsp], 0
	jmp	SHORT $LN10@AuVFSFind
$LN9@AuVFSFind:
	mov	eax, DWORD PTR i$[rsp]
	inc	eax
	mov	DWORD PTR i$[rsp], eax
$LN10@AuVFSFind:
	cmp	DWORD PTR i$[rsp], 16
	jge	SHORT $LN8@AuVFSFind

; 78   : 		if (next[i] == '/' || next[i] == '\0')

	movsxd	rax, DWORD PTR i$[rsp]
	mov	rcx, QWORD PTR next$[rsp]
	movsx	eax, BYTE PTR [rcx+rax]
	cmp	eax, 47					; 0000002fH
	je	SHORT $LN6@AuVFSFind
	movsxd	rax, DWORD PTR i$[rsp]
	mov	rcx, QWORD PTR next$[rsp]
	movsx	eax, BYTE PTR [rcx+rax]
	test	eax, eax
	jne	SHORT $LN7@AuVFSFind
$LN6@AuVFSFind:

; 79   : 			break;

	jmp	SHORT $LN8@AuVFSFind
$LN7@AuVFSFind:

; 80   : 		pathname[i] = next[i];

	movsxd	rax, DWORD PTR i$[rsp]
	movsxd	rcx, DWORD PTR i$[rsp]
	mov	rdx, QWORD PTR next$[rsp]
	movzx	eax, BYTE PTR [rdx+rax]
	mov	BYTE PTR pathname$[rsp+rcx], al

; 81   : 	}

	jmp	SHORT $LN9@AuVFSFind
$LN8@AuVFSFind:

; 82   : 	pathname[i] = 0;

	movsxd	rax, DWORD PTR i$[rsp]
	mov	BYTE PTR pathname$[rsp+rax], 0

; 83   : 
; 84   : 	for (int j = 0; j < __RootContainer->childs->pointer; j++) {

	mov	DWORD PTR j$1[rsp], 0
	jmp	SHORT $LN5@AuVFSFind
$LN4@AuVFSFind:
	mov	eax, DWORD PTR j$1[rsp]
	inc	eax
	mov	DWORD PTR j$1[rsp], eax
$LN5@AuVFSFind:
	mov	rax, QWORD PTR ?__RootContainer@@3PEAU__VFS_Container__@@EA ; __RootContainer
	mov	rax, QWORD PTR [rax]
	mov	eax, DWORD PTR [rax]
	cmp	DWORD PTR j$1[rsp], eax
	jae	SHORT $LN3@AuVFSFind

; 85   : 		AuVFSNode* node = (AuVFSNode*)list_get_at(__RootContainer->childs, j);

	mov	edx, DWORD PTR j$1[rsp]
	mov	rax, QWORD PTR ?__RootContainer@@3PEAU__VFS_Container__@@EA ; __RootContainer
	mov	rcx, QWORD PTR [rax]
	call	list_get_at
	mov	QWORD PTR node$2[rsp], rax

; 86   : 		if ((strcmp(node->filename, pathname) == 0) && (node->flags & FS_FLAG_FILE_SYSTEM)) {

	mov	rax, QWORD PTR node$2[rsp]
	lea	rdx, QWORD PTR pathname$[rsp]
	mov	rcx, rax
	call	strcmp
	test	eax, eax
	jne	SHORT $LN2@AuVFSFind
	mov	rax, QWORD PTR node$2[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 64					; 00000040H
	test	eax, eax
	je	SHORT $LN2@AuVFSFind

; 87   : 			Returnable = node;

	mov	rax, QWORD PTR node$2[rsp]
	mov	QWORD PTR Returnable$[rsp], rax

; 88   : 			break;

	jmp	SHORT $LN3@AuVFSFind
$LN2@AuVFSFind:

; 89   : 		}
; 90   : 	}

	jmp	SHORT $LN4@AuVFSFind
$LN3@AuVFSFind:

; 91   : 
; 92   : 	if (!Returnable)

	cmp	QWORD PTR Returnable$[rsp], 0
	jne	SHORT $LN1@AuVFSFind

; 93   : 		Returnable = __RootFS;

	mov	rax, QWORD PTR ?__RootFS@@3PEAU__VFS_NODE__@@EA ; __RootFS
	mov	QWORD PTR Returnable$[rsp], rax
$LN1@AuVFSFind:

; 94   : 
; 95   : 	return Returnable;

	mov	rax, QWORD PTR Returnable$[rsp]

; 96   : }

	add	rsp, 88					; 00000058H
	ret	0
AuVFSFind ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
fs$ = 8
AuVFSRegisterRoot PROC

; 119  : void AuVFSRegisterRoot(AuVFSNode *fs) {

	mov	QWORD PTR [rsp+8], rcx

; 120  : 	if (__RootFS)

	cmp	QWORD PTR ?__RootFS@@3PEAU__VFS_NODE__@@EA, 0 ; __RootFS
	je	SHORT $LN1@AuVFSRegis

; 121  : 		return;

	jmp	SHORT $LN2@AuVFSRegis
$LN1@AuVFSRegis:

; 122  : 	__RootFS = fs;

	mov	rax, QWORD PTR fs$[rsp]
	mov	QWORD PTR ?__RootFS@@3PEAU__VFS_NODE__@@EA, rax ; __RootFS
$LN2@AuVFSRegis:

; 123  : }

	ret	0
AuVFSRegisterRoot ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
i$1 = 32
node_$2 = 40
node$ = 64
AuVFSAddFileSystem PROC

; 104  : AU_EXTERN AU_EXPORT void AuVFSAddFileSystem(AuVFSNode* node) {

$LN7:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 105  : 	for (int i = 0; i < __RootContainer->childs->pointer; i++) {

	mov	DWORD PTR i$1[rsp], 0
	jmp	SHORT $LN4@AuVFSAddFi
$LN3@AuVFSAddFi:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN4@AuVFSAddFi:
	mov	rax, QWORD PTR ?__RootContainer@@3PEAU__VFS_Container__@@EA ; __RootContainer
	mov	rax, QWORD PTR [rax]
	mov	eax, DWORD PTR [rax]
	cmp	DWORD PTR i$1[rsp], eax
	jae	SHORT $LN2@AuVFSAddFi

; 106  : 		AuVFSNode* node_ = (AuVFSNode*)list_get_at(__RootContainer->childs, i);

	mov	edx, DWORD PTR i$1[rsp]
	mov	rax, QWORD PTR ?__RootContainer@@3PEAU__VFS_Container__@@EA ; __RootContainer
	mov	rcx, QWORD PTR [rax]
	call	list_get_at
	mov	QWORD PTR node_$2[rsp], rax

; 107  : 		if (node_ == node)

	mov	rax, QWORD PTR node$[rsp]
	cmp	QWORD PTR node_$2[rsp], rax
	jne	SHORT $LN1@AuVFSAddFi

; 108  : 			return;

	jmp	SHORT $LN5@AuVFSAddFi
$LN1@AuVFSAddFi:

; 109  : 	}

	jmp	SHORT $LN3@AuVFSAddFi
$LN2@AuVFSAddFi:

; 110  : 	list_add(__RootContainer->childs, node);

	mov	rdx, QWORD PTR node$[rsp]
	mov	rax, QWORD PTR ?__RootContainer@@3PEAU__VFS_Container__@@EA ; __RootContainer
	mov	rcx, QWORD PTR [rax]
	call	list_add
$LN5@AuVFSAddFi:

; 111  : }

	add	rsp, 56					; 00000038H
	ret	0
AuVFSAddFileSystem ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
val$1 = 32
node$ = 64
code$ = 72
arg$ = 80
AuVFSNodeIOControl PROC

; 177  : AU_EXTERN AU_EXPORT int AuVFSNodeIOControl(AuVFSNode* node, int code, void* arg) {

$LN4:
	mov	QWORD PTR [rsp+24], r8
	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 178  : 	if (node->iocontrol){

	mov	rax, QWORD PTR node$[rsp]
	cmp	QWORD PTR [rax+152], 0
	je	SHORT $LN1@AuVFSNodeI

; 179  : 		int val = node->iocontrol(node, code, arg);

	mov	r8, QWORD PTR arg$[rsp]
	mov	edx, DWORD PTR code$[rsp]
	mov	rcx, QWORD PTR node$[rsp]
	mov	rax, QWORD PTR node$[rsp]
	call	QWORD PTR [rax+152]
	mov	DWORD PTR val$1[rsp], eax

; 180  : 		return val;

	mov	eax, DWORD PTR val$1[rsp]
	jmp	SHORT $LN2@AuVFSNodeI
$LN1@AuVFSNodeI:

; 181  : 	}
; 182  : 	return 0;

	xor	eax, eax
$LN2@AuVFSNodeI:

; 183  : }

	add	rsp, 56					; 00000038H
	ret	0
AuVFSNodeIOControl ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
i$1 = 32
next$2 = 40
fs$ = 48
next$3 = 56
Returnable$ = 64
pathname$4 = 72
path$ = 112
AuVFSOpen PROC

; 130  : AU_EXTERN AU_EXPORT AuVFSNode* AuVFSOpen(char* path){

$LN16:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 104				; 00000068H

; 131  : 	AuVFSNode *Returnable = NULL;

	mov	QWORD PTR Returnable$[rsp], 0

; 132  : 	AuVFSNode* fs = AuVFSFind(path);

	mov	rcx, QWORD PTR path$[rsp]
	call	AuVFSFind
	mov	QWORD PTR fs$[rsp], rax

; 133  : 	
; 134  : 	if (!fs)

	cmp	QWORD PTR fs$[rsp], 0
	jne	SHORT $LN13@AuVFSOpen

; 135  : 		return NULL;

	xor	eax, eax
	jmp	$LN14@AuVFSOpen
$LN13@AuVFSOpen:

; 136  : 	if (fs == __RootFS) {

	mov	rax, QWORD PTR ?__RootFS@@3PEAU__VFS_NODE__@@EA ; __RootFS
	cmp	QWORD PTR fs$[rsp], rax
	jne	SHORT $LN12@AuVFSOpen

; 137  : 		
; 138  : 		/* just skip the '/' from the path */
; 139  : 		char* next = strchr(path, '/');

	mov	edx, 47					; 0000002fH
	mov	rcx, QWORD PTR path$[rsp]
	call	strchr
	mov	QWORD PTR next$3[rsp], rax

; 140  : 		if (next)

	cmp	QWORD PTR next$3[rsp], 0
	je	SHORT $LN11@AuVFSOpen

; 141  : 			next++;

	mov	rax, QWORD PTR next$3[rsp]
	inc	rax
	mov	QWORD PTR next$3[rsp], rax
$LN11@AuVFSOpen:

; 142  : 
; 143  : 		if (fs->open)

	mov	rax, QWORD PTR fs$[rsp]
	cmp	QWORD PTR [rax+72], 0
	je	SHORT $LN10@AuVFSOpen

; 144  : 			Returnable = fs->open(fs, path);

	mov	rdx, QWORD PTR path$[rsp]
	mov	rcx, QWORD PTR fs$[rsp]
	mov	rax, QWORD PTR fs$[rsp]
	call	QWORD PTR [rax+72]
	mov	QWORD PTR Returnable$[rsp], rax
$LN10@AuVFSOpen:

; 145  : 
; 146  : 	}
; 147  : 	else {

	jmp	$LN9@AuVFSOpen
$LN12@AuVFSOpen:

; 148  : 		char* next = strchr(path, '/');

	mov	edx, 47					; 0000002fH
	mov	rcx, QWORD PTR path$[rsp]
	call	strchr
	mov	QWORD PTR next$2[rsp], rax

; 149  : 		if (next)

	cmp	QWORD PTR next$2[rsp], 0
	je	SHORT $LN8@AuVFSOpen

; 150  : 			next++;

	mov	rax, QWORD PTR next$2[rsp]
	inc	rax
	mov	QWORD PTR next$2[rsp], rax
$LN8@AuVFSOpen:

; 151  : 
; 152  : 		char pathname[16];
; 153  : 		int i = 0;

	mov	DWORD PTR i$1[rsp], 0

; 154  : 		for (i = 0; i < 16; i++) {

	mov	DWORD PTR i$1[rsp], 0
	jmp	SHORT $LN7@AuVFSOpen
$LN6@AuVFSOpen:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN7@AuVFSOpen:
	cmp	DWORD PTR i$1[rsp], 16
	jge	SHORT $LN5@AuVFSOpen

; 155  : 			if (next[i] == '/' || next[i] == '\0')

	movsxd	rax, DWORD PTR i$1[rsp]
	mov	rcx, QWORD PTR next$2[rsp]
	movsx	eax, BYTE PTR [rcx+rax]
	cmp	eax, 47					; 0000002fH
	je	SHORT $LN3@AuVFSOpen
	movsxd	rax, DWORD PTR i$1[rsp]
	mov	rcx, QWORD PTR next$2[rsp]
	movsx	eax, BYTE PTR [rcx+rax]
	test	eax, eax
	jne	SHORT $LN4@AuVFSOpen
$LN3@AuVFSOpen:

; 156  : 				break;

	jmp	SHORT $LN5@AuVFSOpen
$LN4@AuVFSOpen:

; 157  : 			pathname[i] = next[i];

	movsxd	rax, DWORD PTR i$1[rsp]
	movsxd	rcx, DWORD PTR i$1[rsp]
	mov	rdx, QWORD PTR next$2[rsp]
	movzx	eax, BYTE PTR [rdx+rax]
	mov	BYTE PTR pathname$4[rsp+rcx], al

; 158  : 		}

	jmp	SHORT $LN6@AuVFSOpen
$LN5@AuVFSOpen:

; 159  : 		pathname[i] = 0;

	movsxd	rax, DWORD PTR i$1[rsp]
	mov	BYTE PTR pathname$4[rsp+rax], 0

; 160  : 
; 161  : 		/* skip the fs filename, from the path
; 162  : 		 * and just pass the required path */
; 163  : 		if (strcmp(fs->filename, pathname) == 0) 

	mov	rax, QWORD PTR fs$[rsp]
	lea	rdx, QWORD PTR pathname$4[rsp]
	mov	rcx, rax
	call	strcmp
	test	eax, eax
	jne	SHORT $LN2@AuVFSOpen

; 164  : 			next += i;

	movsxd	rax, DWORD PTR i$1[rsp]
	mov	rcx, QWORD PTR next$2[rsp]
	add	rcx, rax
	mov	rax, rcx
	mov	QWORD PTR next$2[rsp], rax
$LN2@AuVFSOpen:

; 165  : 		if (fs->open)

	mov	rax, QWORD PTR fs$[rsp]
	cmp	QWORD PTR [rax+72], 0
	je	SHORT $LN1@AuVFSOpen

; 166  : 			Returnable = fs->open(fs,next);

	mov	rdx, QWORD PTR next$2[rsp]
	mov	rcx, QWORD PTR fs$[rsp]
	mov	rax, QWORD PTR fs$[rsp]
	call	QWORD PTR [rax+72]
	mov	QWORD PTR Returnable$[rsp], rax
$LN1@AuVFSOpen:
$LN9@AuVFSOpen:

; 167  : 	}
; 168  : 	return Returnable;

	mov	rax, QWORD PTR Returnable$[rsp]
$LN14@AuVFSOpen:

; 169  : }

	add	rsp, 104				; 00000068H
	ret	0
AuVFSOpen ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\fs\vfs.cpp
_TEXT	SEGMENT
_root$ = 32
?AuVFSInitialise@@YAXXZ PROC				; AuVFSInitialise

; 46   : void AuVFSInitialise() {

$LN3:
	sub	rsp, 56					; 00000038H

; 47   : 	AuVFSContainer* _root = (AuVFSContainer*)kmalloc(sizeof(AuVFSContainer));

	mov	ecx, 8
	call	kmalloc
	mov	QWORD PTR _root$[rsp], rax

; 48   : 	_root->childs = initialize_list();

	call	initialize_list
	mov	rcx, QWORD PTR _root$[rsp]
	mov	QWORD PTR [rcx], rax

; 49   : 	__RootContainer = _root;

	mov	rax, QWORD PTR _root$[rsp]
	mov	QWORD PTR ?__RootContainer@@3PEAU__VFS_Container__@@EA, rax ; __RootContainer

; 50   : 	__RootFS = NULL;

	mov	QWORD PTR ?__RootFS@@3PEAU__VFS_NODE__@@EA, 0 ; __RootFS

; 51   : 	_vfs_debug_on = false;

	mov	BYTE PTR ?_vfs_debug_on@@3_NA, 0	; _vfs_debug_on

; 52   : 	/* initialise the device file system */
; 53   : 	AuDeviceFsInitialize();

	call	?AuDeviceFsInitialize@@YAXXZ		; AuDeviceFsInitialize

; 54   : 	/* here we need to mount the
; 55   : 	 * root file system
; 56   : 	 */
; 57   : }

	add	rsp, 56					; 00000038H
	ret	0
?AuVFSInitialise@@YAXXZ ENDP				; AuVFSInitialise
_TEXT	ENDS
END