; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

CONST	SEGMENT
$SG3871	DB	'File Closed -> %s %x ', 0dH, 0aH, 00H
CONST	ENDS
PUBLIC	?OpenFile@@YAHPEADH@Z				; OpenFile
PUBLIC	?ReadFile@@YA_KHPEAX_K@Z			; ReadFile
PUBLIC	?WriteFile@@YA_KHPEAX_K@Z			; WriteFile
PUBLIC	?CreateDir@@YAHPEAD@Z				; CreateDir
PUBLIC	?RemoveFile@@YAHPEAD@Z				; RemoveFile
PUBLIC	?CloseFile@@YAHH@Z				; CloseFile
PUBLIC	?FileIoControl@@YAHHHPEAX@Z			; FileIoControl
PUBLIC	?FileStat@@YAHHPEAX@Z				; FileStat
EXTRN	AuVFSOpen:PROC
EXTRN	AuVFSNodeIOControl:PROC
EXTRN	AuVFSFind:PROC
EXTRN	AuVFSNodeRead:PROC
EXTRN	AuVFSCreateDir:PROC
EXTRN	AuVFSCreateFile:PROC
EXTRN	?AuVFSRemoveFile@@YAHPEAU__VFS_NODE__@@0@Z:PROC	; AuVFSRemoveFile
EXTRN	?AuVFSRemoveDir@@YAHPEAU__VFS_NODE__@@0@Z:PROC	; AuVFSRemoveDir
EXTRN	AuVFSNodeWrite:PROC
EXTRN	AuGetCurrentThread:PROC
EXTRN	AuPmmngrAlloc:PROC
EXTRN	AuPmmngrFree:PROC
EXTRN	P2V:PROC
EXTRN	V2P:PROC
EXTRN	kfree:PROC
EXTRN	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z:PROC ; AuProcessFindThread
EXTRN	?AuProcessGetFileDesc@@YAHPEAU_au_proc_@@@Z:PROC ; AuProcessGetFileDesc
EXTRN	memset:PROC
EXTRN	memcpy:PROC
EXTRN	SeTextOut:PROC
EXTRN	x64_cli:PROC
pdata	SEGMENT
$pdata$?OpenFile@@YAHPEADH@Z DD imagerel $LN9
	DD	imagerel $LN9+207
	DD	imagerel $unwind$?OpenFile@@YAHPEADH@Z
$pdata$?ReadFile@@YA_KHPEAX_K@Z DD imagerel $LN12
	DD	imagerel $LN12+343
	DD	imagerel $unwind$?ReadFile@@YA_KHPEAX_K@Z
$pdata$?WriteFile@@YA_KHPEAX_K@Z DD imagerel $LN10
	DD	imagerel $LN10+349
	DD	imagerel $unwind$?WriteFile@@YA_KHPEAX_K@Z
$pdata$?CreateDir@@YAHPEAD@Z DD imagerel $LN6
	DD	imagerel $LN6+127
	DD	imagerel $unwind$?CreateDir@@YAHPEAD@Z
$pdata$?RemoveFile@@YAHPEAD@Z DD imagerel $LN6
	DD	imagerel $LN6+108
	DD	imagerel $unwind$?RemoveFile@@YAHPEAD@Z
$pdata$?CloseFile@@YAHH@Z DD imagerel $LN6
	DD	imagerel $LN6+173
	DD	imagerel $unwind$?CloseFile@@YAHH@Z
$pdata$?FileIoControl@@YAHHHPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+134
	DD	imagerel $unwind$?FileIoControl@@YAHHHPEAX@Z
$pdata$?FileStat@@YAHHPEAX@Z DD imagerel $LN5
	DD	imagerel $LN5+216
	DD	imagerel $unwind$?FileStat@@YAHHPEAX@Z
pdata	ENDS
xdata	SEGMENT
$unwind$?OpenFile@@YAHPEADH@Z DD 010d01H
	DD	0a20dH
$unwind$?ReadFile@@YA_KHPEAX_K@Z DD 011201H
	DD	0a212H
$unwind$?WriteFile@@YA_KHPEAX_K@Z DD 011201H
	DD	0c212H
$unwind$?CreateDir@@YAHPEAD@Z DD 010901H
	DD	08209H
$unwind$?RemoveFile@@YAHPEAD@Z DD 010901H
	DD	06209H
$unwind$?CloseFile@@YAHH@Z DD 010801H
	DD	08208H
$unwind$?FileIoControl@@YAHHHPEAX@Z DD 011101H
	DD	08211H
$unwind$?FileStat@@YAHHPEAX@Z DD 010d01H
	DD	0820dH
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\fileserv.cpp
_TEXT	SEGMENT
status$ = 32
file$ = 40
current_thr$ = 48
current_proc$ = 56
fd$ = 80
buf$ = 88
?FileStat@@YAHHPEAX@Z PROC				; FileStat

; 245  : int FileStat(int fd, void* buf) {

$LN5:
	mov	QWORD PTR [rsp+16], rdx
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 72					; 00000048H

; 246  : 	if (fd == -1)

	cmp	DWORD PTR fd$[rsp], -1
	jne	SHORT $LN2@FileStat

; 247  : 		return -1;

	mov	eax, -1
	jmp	$LN3@FileStat
$LN2@FileStat:

; 248  : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 249  : 	AuProcess* current_proc = AuProcessFindThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR current_proc$[rsp], rax

; 250  : 	AuVFSNode* file = current_proc->fds[fd];

	movsxd	rax, DWORD PTR fd$[rsp]
	mov	rcx, QWORD PTR current_proc$[rsp]
	mov	rax, QWORD PTR [rcx+rax*8+551]
	mov	QWORD PTR file$[rsp], rax

; 251  : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN1@FileStat

; 252  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN3@FileStat
$LN1@FileStat:

; 253  : 
; 254  : 	AuFileStatus *status = (AuFileStatus*)buf;

	mov	rax, QWORD PTR buf$[rsp]
	mov	QWORD PTR status$[rsp], rax

; 255  : 	status->current_block = file->current;

	mov	rax, QWORD PTR status$[rsp]
	mov	rcx, QWORD PTR file$[rsp]
	mov	ecx, DWORD PTR [rcx+53]
	mov	DWORD PTR [rax+9], ecx

; 256  : 	status->size = file->size;

	mov	rax, QWORD PTR file$[rsp]
	mov	eax, DWORD PTR [rax+32]
	mov	rcx, QWORD PTR status$[rsp]
	mov	QWORD PTR [rcx+1], rax

; 257  : 	status->filemode = file->flags;

	mov	rax, QWORD PTR status$[rsp]
	mov	rcx, QWORD PTR file$[rsp]
	movzx	ecx, BYTE PTR [rcx+61]
	mov	BYTE PTR [rax], cl

; 258  : 	status->eof = file->eof;

	mov	rax, QWORD PTR status$[rsp]
	mov	rcx, QWORD PTR file$[rsp]
	movzx	ecx, BYTE PTR [rcx+36]
	mov	BYTE PTR [rax+29], cl

; 259  : 	status->start_block = file->first_block;

	mov	rax, QWORD PTR status$[rsp]
	mov	rcx, QWORD PTR file$[rsp]
	mov	ecx, DWORD PTR [rcx+45]
	mov	DWORD PTR [rax+13], ecx

; 260  : 	status->user_id = 0;

	mov	rax, QWORD PTR status$[rsp]
	mov	DWORD PTR [rax+17], 0

; 261  : 	status->group_id = 0;

	mov	rax, QWORD PTR status$[rsp]
	mov	DWORD PTR [rax+21], 0

; 262  : 	return 0;

	xor	eax, eax
$LN3@FileStat:

; 263  : }

	add	rsp, 72					; 00000048H
	ret	0
?FileStat@@YAHHPEAX@Z ENDP				; FileStat
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\fileserv.cpp
_TEXT	SEGMENT
ret$ = 32
file$ = 40
current_thr$ = 48
current_proc$ = 56
fd$ = 80
code$ = 88
arg$ = 96
?FileIoControl@@YAHHHPEAX@Z PROC			; FileIoControl

; 224  : int FileIoControl(int fd, int code, void* arg) {

$LN5:
	mov	QWORD PTR [rsp+24], r8
	mov	DWORD PTR [rsp+16], edx
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 72					; 00000048H

; 225  : 	if (fd == -1)

	cmp	DWORD PTR fd$[rsp], -1
	jne	SHORT $LN2@FileIoCont

; 226  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN3@FileIoCont
$LN2@FileIoCont:

; 227  : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 228  : 	AuProcess* current_proc = AuProcessFindThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR current_proc$[rsp], rax

; 229  : 	AuVFSNode* file = current_proc->fds[fd];

	movsxd	rax, DWORD PTR fd$[rsp]
	mov	rcx, QWORD PTR current_proc$[rsp]
	mov	rax, QWORD PTR [rcx+rax*8+551]
	mov	QWORD PTR file$[rsp], rax

; 230  : 
; 231  : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN1@FileIoCont

; 232  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN3@FileIoCont
$LN1@FileIoCont:

; 233  : 
; 234  : 	int ret = 0;

	mov	DWORD PTR ret$[rsp], 0

; 235  : 	ret = AuVFSNodeIOControl(file, code, arg);

	mov	r8, QWORD PTR arg$[rsp]
	mov	edx, DWORD PTR code$[rsp]
	mov	rcx, QWORD PTR file$[rsp]
	call	AuVFSNodeIOControl
	mov	DWORD PTR ret$[rsp], eax

; 236  : 	return ret;

	mov	eax, DWORD PTR ret$[rsp]
$LN3@FileIoCont:

; 237  : }

	add	rsp, 72					; 00000048H
	ret	0
?FileIoControl@@YAHHHPEAX@Z ENDP			; FileIoControl
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\fileserv.cpp
_TEXT	SEGMENT
file$ = 32
current_proc$ = 40
current_thr$ = 48
fd$ = 80
?CloseFile@@YAHH@Z PROC					; CloseFile

; 202  : int CloseFile(int fd) {

$LN6:
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 72					; 00000048H

; 203  : 	if (fd == -1)

	cmp	DWORD PTR fd$[rsp], -1
	jne	SHORT $LN3@CloseFile

; 204  : 		return 0;

	xor	eax, eax
	jmp	$LN4@CloseFile
$LN3@CloseFile:

; 205  : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 206  : 	AuProcess* current_proc = AuProcessFindThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR current_proc$[rsp], rax

; 207  : 	AuVFSNode* file = current_proc->fds[fd];

	movsxd	rax, DWORD PTR fd$[rsp]
	mov	rcx, QWORD PTR current_proc$[rsp]
	mov	rax, QWORD PTR [rcx+rax*8+551]
	mov	QWORD PTR file$[rsp], rax

; 208  : 	SeTextOut("File Closed -> %s %x \r\n", file->filename, file);

	mov	rax, QWORD PTR file$[rsp]
	mov	r8, QWORD PTR file$[rsp]
	mov	rdx, rax
	lea	rcx, OFFSET FLAT:$SG3871
	call	SeTextOut

; 209  : 	if (file->flags & FS_FLAG_FILE_SYSTEM)

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 64					; 00000040H
	test	eax, eax
	je	SHORT $LN2@CloseFile

; 210  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN4@CloseFile
$LN2@CloseFile:

; 211  : 	if (file->flags & FS_FLAG_GENERAL)

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 4
	test	eax, eax
	je	SHORT $LN1@CloseFile

; 212  : 		kfree(file);

	mov	rcx, QWORD PTR file$[rsp]
	call	kfree
$LN1@CloseFile:

; 213  : 
; 214  : 	current_proc->fds[fd] = 0;

	movsxd	rax, DWORD PTR fd$[rsp]
	mov	rcx, QWORD PTR current_proc$[rsp]
	mov	QWORD PTR [rcx+rax*8+551], 0

; 215  : 	return 0;

	xor	eax, eax
$LN4@CloseFile:

; 216  : }

	add	rsp, 72					; 00000048H
	ret	0
?CloseFile@@YAHH@Z ENDP					; CloseFile
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\fileserv.cpp
_TEXT	SEGMENT
dir$ = 32
fsys$ = 40
pathname$ = 64
?RemoveFile@@YAHPEAD@Z PROC				; RemoveFile

; 187  : int RemoveFile(char* pathname) {

$LN6:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 188  : 	AuVFSNode* dir = AuVFSOpen(pathname);

	mov	rcx, QWORD PTR pathname$[rsp]
	call	AuVFSOpen
	mov	QWORD PTR dir$[rsp], rax

; 189  : 	if (!dir)

	cmp	QWORD PTR dir$[rsp], 0
	jne	SHORT $LN3@RemoveFile

; 190  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN4@RemoveFile
$LN3@RemoveFile:

; 191  : 	AuVFSNode* fsys = (AuVFSNode*)dir->device;

	mov	rax, QWORD PTR dir$[rsp]
	mov	rax, QWORD PTR [rax+64]
	mov	QWORD PTR fsys$[rsp], rax

; 192  : 	if (fsys->flags & FS_FLAG_DIRECTORY)

	mov	rax, QWORD PTR fsys$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 2
	test	eax, eax
	je	SHORT $LN2@RemoveFile

; 193  : 		return AuVFSRemoveDir(fsys, dir);

	mov	rdx, QWORD PTR dir$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	call	?AuVFSRemoveDir@@YAHPEAU__VFS_NODE__@@0@Z ; AuVFSRemoveDir
	jmp	SHORT $LN4@RemoveFile

; 194  : 	else

	jmp	SHORT $LN1@RemoveFile
$LN2@RemoveFile:

; 195  : 		return AuVFSRemoveFile(fsys, dir);

	mov	rdx, QWORD PTR dir$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	call	?AuVFSRemoveFile@@YAHPEAU__VFS_NODE__@@0@Z ; AuVFSRemoveFile
$LN1@RemoveFile:
$LN4@RemoveFile:

; 196  : }

	add	rsp, 56					; 00000038H
	ret	0
?RemoveFile@@YAHPEAD@Z ENDP				; RemoveFile
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\fileserv.cpp
_TEXT	SEGMENT
dirfile$ = 32
fsys$ = 40
current_thr$ = 48
current_proc$ = 56
filename$ = 80
?CreateDir@@YAHPEAD@Z PROC				; CreateDir

; 162  : int CreateDir(char* filename) {

$LN6:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 72					; 00000048H

; 163  : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 164  : 	AuProcess* current_proc = AuProcessFindThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR current_proc$[rsp], rax

; 165  : 
; 166  : 	AuVFSNode *fsys = AuVFSFind(filename);

	mov	rcx, QWORD PTR filename$[rsp]
	call	AuVFSFind
	mov	QWORD PTR fsys$[rsp], rax

; 167  : 	AuVFSNode* dirfile = NULL;

	mov	QWORD PTR dirfile$[rsp], 0

; 168  : 	if (fsys){

	cmp	QWORD PTR fsys$[rsp], 0
	je	SHORT $LN3@CreateDir

; 169  : 		dirfile = AuVFSCreateDir(fsys, filename);

	mov	rdx, QWORD PTR filename$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	call	AuVFSCreateDir
	mov	QWORD PTR dirfile$[rsp], rax

; 170  : 	}
; 171  : 	else {

	jmp	SHORT $LN2@CreateDir
$LN3@CreateDir:

; 172  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN4@CreateDir
$LN2@CreateDir:

; 173  : 	}
; 174  : 
; 175  : 	if (dirfile) {

	cmp	QWORD PTR dirfile$[rsp], 0
	je	SHORT $LN1@CreateDir

; 176  : 		kfree(dirfile);

	mov	rcx, QWORD PTR dirfile$[rsp]
	call	kfree

; 177  : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN4@CreateDir
$LN1@CreateDir:

; 178  : 	}
; 179  : 
; 180  : 	return -1;

	mov	eax, -1
$LN4@CreateDir:

; 181  : }

	add	rsp, 72					; 00000048H
	ret	0
?CreateDir@@YAHPEAD@Z ENDP				; CreateDir
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\fileserv.cpp
_TEXT	SEGMENT
file$ = 32
buff$1 = 40
fsys$ = 48
current_thr$ = 56
current_proc$ = 64
aligned_buffer$ = 72
write_bytes$ = 80
fd$ = 112
buffer$ = 120
length$ = 128
?WriteFile@@YA_KHPEAX_K@Z PROC				; WriteFile

; 123  : size_t WriteFile(int fd, void* buffer, size_t length) {

$LN10:
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 104				; 00000068H

; 124  : 	x64_cli();

	call	x64_cli

; 125  : 	if (fd == -1)

	cmp	DWORD PTR fd$[rsp], -1
	jne	SHORT $LN7@WriteFile

; 126  : 		return 0;

	xor	eax, eax
	jmp	$LN8@WriteFile
$LN7@WriteFile:

; 127  : 	if (!buffer)

	cmp	QWORD PTR buffer$[rsp], 0
	jne	SHORT $LN6@WriteFile

; 128  : 		return 0;

	xor	eax, eax
	jmp	$LN8@WriteFile
$LN6@WriteFile:

; 129  : 	if (!length)

	cmp	QWORD PTR length$[rsp], 0
	jne	SHORT $LN5@WriteFile

; 130  : 		return 0;

	xor	eax, eax
	jmp	$LN8@WriteFile
$LN5@WriteFile:

; 131  : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 132  : 	AuProcess* current_proc = AuProcessFindThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR current_proc$[rsp], rax

; 133  : 	AuVFSNode* file = current_proc->fds[fd];

	movsxd	rax, DWORD PTR fd$[rsp]
	mov	rcx, QWORD PTR current_proc$[rsp]
	mov	rax, QWORD PTR [rcx+rax*8+551]
	mov	QWORD PTR file$[rsp], rax

; 134  : 	uint8_t* aligned_buffer = (uint8_t*)buffer;

	mov	rax, QWORD PTR buffer$[rsp]
	mov	QWORD PTR aligned_buffer$[rsp], rax

; 135  : 
; 136  : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN4@WriteFile

; 137  : 		return 0;

	xor	eax, eax
	jmp	$LN8@WriteFile
$LN4@WriteFile:

; 138  : 	size_t write_bytes = 0;

	mov	QWORD PTR write_bytes$[rsp], 0

; 139  : 	size_t ret_bytes;
; 140  : 	/* every general file will contain its
; 141  : 	* file system node as device */
; 142  : 	AuVFSNode* fsys = (AuVFSNode*)file->device;

	mov	rax, QWORD PTR file$[rsp]
	mov	rax, QWORD PTR [rax+64]
	mov	QWORD PTR fsys$[rsp], rax

; 143  : 	if (file->flags & FS_FLAG_GENERAL) {

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 4
	test	eax, eax
	je	SHORT $LN3@WriteFile

; 144  : 		uint64_t* buff = (uint64_t*)P2V((size_t)AuPmmngrAlloc());

	call	AuPmmngrAlloc
	mov	rcx, rax
	call	P2V
	mov	QWORD PTR buff$1[rsp], rax

; 145  : 		memset(buff, 0, PAGE_SIZE);

	mov	r8d, 4096				; 00001000H
	xor	edx, edx
	mov	rcx, QWORD PTR buff$1[rsp]
	call	memset

; 146  : 		memcpy(buff,aligned_buffer, PAGE_SIZE);

	mov	r8d, 4096				; 00001000H
	mov	rdx, QWORD PTR aligned_buffer$[rsp]
	mov	rcx, QWORD PTR buff$1[rsp]
	call	memcpy

; 147  : 		AuVFSNodeWrite(fsys, file, buff, length);

	mov	r9d, DWORD PTR length$[rsp]
	mov	r8, QWORD PTR buff$1[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	call	AuVFSNodeWrite

; 148  : 		AuPmmngrFree((void*)V2P((size_t)buff));

	mov	rcx, QWORD PTR buff$1[rsp]
	call	V2P
	mov	rcx, rax
	call	AuPmmngrFree
$LN3@WriteFile:

; 149  : 	}
; 150  : 
; 151  : 	if (file->flags & FS_FLAG_DEVICE) {

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 8
	test	eax, eax
	je	SHORT $LN2@WriteFile

; 152  : 		if (file->write) {

	mov	rax, QWORD PTR file$[rsp]
	cmp	QWORD PTR [rax+88], 0
	je	SHORT $LN1@WriteFile

; 153  : 			file->write(fsys, file, (uint64_t*)buffer, length);

	mov	r9d, DWORD PTR length$[rsp]
	mov	r8, QWORD PTR buffer$[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	mov	rax, QWORD PTR file$[rsp]
	call	QWORD PTR [rax+88]
$LN1@WriteFile:
$LN2@WriteFile:
$LN8@WriteFile:

; 154  : 		}
; 155  : 	}
; 156  : }

	add	rsp, 104				; 00000068H
	ret	0
?WriteFile@@YA_KHPEAX_K@Z ENDP				; WriteFile
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\fileserv.cpp
_TEXT	SEGMENT
file$ = 32
ret_bytes$ = 40
current_thr$ = 48
current_proc$ = 56
aligned_buffer$ = 64
fsys$ = 72
fd$ = 96
buffer$ = 104
length$ = 112
?ReadFile@@YA_KHPEAX_K@Z PROC				; ReadFile

; 80   : size_t ReadFile(int fd, void* buffer, size_t length) {

$LN12:
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 88					; 00000058H

; 81   : 	x64_cli();

	call	x64_cli

; 82   : 	if (fd == -1)

	cmp	DWORD PTR fd$[rsp], -1
	jne	SHORT $LN9@ReadFile

; 83   : 		return 0;

	xor	eax, eax
	jmp	$LN10@ReadFile
$LN9@ReadFile:

; 84   : 	if (!buffer)

	cmp	QWORD PTR buffer$[rsp], 0
	jne	SHORT $LN8@ReadFile

; 85   : 		return 0;

	xor	eax, eax
	jmp	$LN10@ReadFile
$LN8@ReadFile:

; 86   : 	if (!length)

	cmp	QWORD PTR length$[rsp], 0
	jne	SHORT $LN7@ReadFile

; 87   : 		return 0;

	xor	eax, eax
	jmp	$LN10@ReadFile
$LN7@ReadFile:

; 88   : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 89   : 	AuProcess* current_proc = AuProcessFindThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR current_proc$[rsp], rax

; 90   : 	AuVFSNode* file = current_proc->fds[fd];

	movsxd	rax, DWORD PTR fd$[rsp]
	mov	rcx, QWORD PTR current_proc$[rsp]
	mov	rax, QWORD PTR [rcx+rax*8+551]
	mov	QWORD PTR file$[rsp], rax

; 91   : 	uint64_t* aligned_buffer = (uint64_t*)buffer;

	mov	rax, QWORD PTR buffer$[rsp]
	mov	QWORD PTR aligned_buffer$[rsp], rax

; 92   : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN6@ReadFile

; 93   : 		return 0;

	xor	eax, eax
	jmp	$LN10@ReadFile
$LN6@ReadFile:

; 94   : 	size_t ret_bytes = 0;

	mov	QWORD PTR ret_bytes$[rsp], 0

; 95   : 	
; 96   : 	/* every general file will contain its
; 97   : 	 * file system node as device */
; 98   : 	AuVFSNode* fsys = (AuVFSNode*)file->device;

	mov	rax, QWORD PTR file$[rsp]
	mov	rax, QWORD PTR [rax+64]
	mov	QWORD PTR fsys$[rsp], rax

; 99   : 	if (file->flags & FS_FLAG_GENERAL) {

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 4
	test	eax, eax
	je	SHORT $LN5@ReadFile

; 100  : 		ret_bytes = AuVFSNodeRead(fsys, file,aligned_buffer, length);

	mov	r9d, DWORD PTR length$[rsp]
	mov	r8, QWORD PTR aligned_buffer$[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	call	AuVFSNodeRead
	mov	QWORD PTR ret_bytes$[rsp], rax
$LN5@ReadFile:

; 101  : 	}
; 102  : 	if (file->flags & FS_FLAG_DEVICE){

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 8
	test	eax, eax
	je	SHORT $LN4@ReadFile

; 103  : 		/* devfs will handle*/
; 104  : 		if (file->read)

	mov	rax, QWORD PTR file$[rsp]
	cmp	QWORD PTR [rax+80], 0
	je	SHORT $LN3@ReadFile

; 105  : 			ret_bytes = file->read(file, file, (uint64_t*)buffer, length);

	mov	r9d, DWORD PTR length$[rsp]
	mov	r8, QWORD PTR buffer$[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR file$[rsp]
	mov	rax, QWORD PTR file$[rsp]
	call	QWORD PTR [rax+80]
	mov	QWORD PTR ret_bytes$[rsp], rax
$LN3@ReadFile:
$LN4@ReadFile:

; 106  : 	}
; 107  : 	if ((file->flags & FS_FLAG_PIPE)) {

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+61]
	and	eax, 128				; 00000080H
	test	eax, eax
	je	SHORT $LN2@ReadFile

; 108  : 		/* ofcourse, pipe subsystem will handle */
; 109  : 		if (file->read)

	mov	rax, QWORD PTR file$[rsp]
	cmp	QWORD PTR [rax+80], 0
	je	SHORT $LN1@ReadFile

; 110  : 			ret_bytes = file->read(file, file, (uint64_t*)buffer, length);

	mov	r9d, DWORD PTR length$[rsp]
	mov	r8, QWORD PTR buffer$[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	rcx, QWORD PTR file$[rsp]
	mov	rax, QWORD PTR file$[rsp]
	call	QWORD PTR [rax+80]
	mov	QWORD PTR ret_bytes$[rsp], rax
$LN1@ReadFile:
$LN2@ReadFile:

; 111  : 	}
; 112  : 
; 113  : 	return ret_bytes;

	mov	rax, QWORD PTR ret_bytes$[rsp]
$LN10@ReadFile:

; 114  : }

	add	rsp, 88					; 00000058H
	ret	0
?ReadFile@@YA_KHPEAX_K@Z ENDP				; ReadFile
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\fileserv.cpp
_TEXT	SEGMENT
fd$ = 32
file$ = 40
current_proc$ = 48
current_thr$ = 56
fsys$ = 64
filename$ = 96
mode$ = 104
?OpenFile@@YAHPEADH@Z PROC				; OpenFile

; 48   : int OpenFile(char* filename, int mode) {

$LN9:
	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 88					; 00000058H

; 49   : 	x64_cli();

	call	x64_cli

; 50   : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 51   : 	AuProcess* current_proc = AuProcessFindThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR current_proc$[rsp], rax

; 52   : 
; 53   : 	AuVFSNode *fsys = AuVFSFind(filename);

	mov	rcx, QWORD PTR filename$[rsp]
	call	AuVFSFind
	mov	QWORD PTR fsys$[rsp], rax

; 54   : 	AuVFSNode* file = AuVFSOpen(filename);

	mov	rcx, QWORD PTR filename$[rsp]
	call	AuVFSOpen
	mov	QWORD PTR file$[rsp], rax

; 55   : 	if (!file) {

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN6@OpenFile

; 56   : 		if (mode & FILE_OPEN_CREAT || mode & FILE_OPEN_WRITE) {

	mov	eax, DWORD PTR mode$[rsp]
	and	eax, 8
	test	eax, eax
	jne	SHORT $LN4@OpenFile
	mov	eax, DWORD PTR mode$[rsp]
	and	eax, 4
	test	eax, eax
	je	SHORT $LN5@OpenFile
$LN4@OpenFile:

; 57   : 			file = AuVFSCreateFile(fsys, filename);

	mov	rdx, QWORD PTR filename$[rsp]
	mov	rcx, QWORD PTR fsys$[rsp]
	call	AuVFSCreateFile
	mov	QWORD PTR file$[rsp], rax

; 58   : 		}
; 59   : 		else 

	jmp	SHORT $LN3@OpenFile
$LN5@OpenFile:

; 60   : 			return -1;

	mov	eax, -1
	jmp	SHORT $LN7@OpenFile
$LN3@OpenFile:
$LN6@OpenFile:

; 61   : 	}
; 62   : 
; 63   : 	/* check for last time, if any error occured */
; 64   : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN2@OpenFile

; 65   : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN7@OpenFile
$LN2@OpenFile:

; 66   : 
; 67   : 	int fd = AuProcessGetFileDesc(current_proc);

	mov	rcx, QWORD PTR current_proc$[rsp]
	call	?AuProcessGetFileDesc@@YAHPEAU_au_proc_@@@Z ; AuProcessGetFileDesc
	mov	DWORD PTR fd$[rsp], eax

; 68   : 	if (fd == -1)

	cmp	DWORD PTR fd$[rsp], -1
	jne	SHORT $LN1@OpenFile

; 69   : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN7@OpenFile
$LN1@OpenFile:

; 70   : 	current_proc->fds[fd] = file;

	movsxd	rax, DWORD PTR fd$[rsp]
	mov	rcx, QWORD PTR current_proc$[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	QWORD PTR [rcx+rax*8+551], rdx

; 71   : 	return fd;

	mov	eax, DWORD PTR fd$[rsp]
$LN7@OpenFile:

; 72   : }

	add	rsp, 88					; 00000058H
	ret	0
?OpenFile@@YAHPEADH@Z ENDP				; OpenFile
_TEXT	ENDS
END