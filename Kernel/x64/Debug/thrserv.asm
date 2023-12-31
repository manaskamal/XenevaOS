; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

CONST	SEGMENT
$SG3798	DB	'No process found ', 0aH, 00H
	ORG $+5
$SG3814	DB	'Process launched failed %s', 0dH, 0aH, 00H
	ORG $+3
$SG3825	DB	'Signal Return ', 0dH, 0aH, 00H
CONST	ENDS
PUBLIC	?PauseThread@@YAHXZ				; PauseThread
PUBLIC	?GetThreadID@@YAGXZ				; GetThreadID
PUBLIC	?GetProcessID@@YAHXZ				; GetProcessID
PUBLIC	?ProcessExit@@YAHXZ				; ProcessExit
PUBLIC	?ProcessWaitForTermination@@YAHH@Z		; ProcessWaitForTermination
PUBLIC	?CreateProcess@@YAHHPEAD@Z			; CreateProcess
PUBLIC	?ProcessLoadExec@@YAHHPEADHPEAPEAD@Z		; ProcessLoadExec
PUBLIC	?ProcessSleep@@YAH_K@Z				; ProcessSleep
PUBLIC	?SignalReturn@@YAXH@Z				; SignalReturn
PUBLIC	?SetSignal@@YAHHP6AXH@Z@Z			; SetSignal
PUBLIC	?GetSystemTimerTick@@YA_KXZ			; GetSystemTimerTick
PUBLIC	?CreateUserThread@@YAHP6AXPEAX@ZPEAD@Z		; CreateUserThread
PUBLIC	?CloseUserThread@@YAHH@Z			; CloseUserThread
PUBLIC	?SetFileToProcess@@YAHHHH@Z			; SetFileToProcess
PUBLIC	?SendSignal@@YAHHH@Z				; SendSignal
PUBLIC	?GetCurrentTime@@YAHPEAX@Z			; GetCurrentTime
EXTRN	AuGetCurrentThread:PROC
EXTRN	AuBlockThread:PROC
EXTRN	AuSleepThread:PROC
EXTRN	AuForceScheduler:PROC
EXTRN	AuGetSystemTimerTick:PROC
EXTRN	?AuProcessFindPID@@YAPEAU_au_proc_@@H@Z:PROC	; AuProcessFindPID
EXTRN	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z:PROC ; AuProcessFindThread
EXTRN	?AuProcessFindSubThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z:PROC ; AuProcessFindSubThread
EXTRN	?AuCreateProcessSlot@@YAPEAU_au_proc_@@PEAU1@PEAD@Z:PROC ; AuCreateProcessSlot
EXTRN	?AuProcessExit@@YAXPEAU_au_proc_@@_N@Z:PROC	; AuProcessExit
EXTRN	?AuProcessWaitForTermination@@YAXPEAU_au_proc_@@H@Z:PROC ; AuProcessWaitForTermination
EXTRN	?AuCreateUserthread@@YAHPEAU_au_proc_@@P6AXPEAX@ZPEAD@Z:PROC ; AuCreateUserthread
EXTRN	?AuSendSignal@@YAXGH@Z:PROC			; AuSendSignal
EXTRN	x64_cli:PROC
EXTRN	x64_force_sched:PROC
EXTRN	strlen:PROC
EXTRN	memset:PROC
EXTRN	SeTextOut:PROC
EXTRN	?AuLoadExecToProcess@@YAHPEAU_au_proc_@@PEADHPEAPEAD@Z:PROC ; AuLoadExecToProcess
EXTRN	kmalloc:PROC
EXTRN	AuGetCurrentTime:PROC
EXTRN	AuTextOut:PROC
pdata	SEGMENT
$pdata$?PauseThread@@YAHXZ DD imagerel $LN3
	DD	imagerel $LN3+44
	DD	imagerel $unwind$?PauseThread@@YAHXZ
$pdata$?GetThreadID@@YAGXZ DD imagerel $LN4
	DD	imagerel $LN4+46
	DD	imagerel $unwind$?GetThreadID@@YAGXZ
$pdata$?GetProcessID@@YAHXZ DD imagerel $LN6
	DD	imagerel $LN6+94
	DD	imagerel $unwind$?GetProcessID@@YAHXZ
$pdata$?ProcessExit@@YAHXZ DD imagerel $LN4
	DD	imagerel $LN4+63
	DD	imagerel $unwind$?ProcessExit@@YAHXZ
$pdata$?ProcessWaitForTermination@@YAHH@Z DD imagerel $LN3
	DD	imagerel $LN3+54
	DD	imagerel $unwind$?ProcessWaitForTermination@@YAHH@Z
$pdata$?CreateProcess@@YAHHPEAD@Z DD imagerel $LN4
	DD	imagerel $LN4+57
	DD	imagerel $unwind$?CreateProcess@@YAHHPEAD@Z
$pdata$?ProcessLoadExec@@YAHHPEADHPEAPEAD@Z DD imagerel $LN12
	DD	imagerel $LN12+332
	DD	imagerel $unwind$?ProcessLoadExec@@YAHHPEADHPEAPEAD@Z
$pdata$?ProcessSleep@@YAH_K@Z DD imagerel $LN5
	DD	imagerel $LN5+92
	DD	imagerel $unwind$?ProcessSleep@@YAH_K@Z
$pdata$?SignalReturn@@YAXH@Z DD imagerel $LN3
	DD	imagerel $LN3+30
	DD	imagerel $unwind$?SignalReturn@@YAXH@Z
$pdata$?SetSignal@@YAHHP6AXH@Z@Z DD imagerel $LN4
	DD	imagerel $LN4+63
	DD	imagerel $unwind$?SetSignal@@YAHHP6AXH@Z@Z
$pdata$?GetSystemTimerTick@@YA_KXZ DD imagerel $LN3
	DD	imagerel $LN3+14
	DD	imagerel $unwind$?GetSystemTimerTick@@YA_KXZ
$pdata$?CreateUserThread@@YAHP6AXPEAX@ZPEAD@Z DD imagerel $LN6
	DD	imagerel $LN6+124
	DD	imagerel $unwind$?CreateUserThread@@YAHP6AXPEAX@ZPEAD@Z
$pdata$?CloseUserThread@@YAHH@Z DD imagerel $LN4
	DD	imagerel $LN4+57
	DD	imagerel $unwind$?CloseUserThread@@YAHH@Z
$pdata$?SetFileToProcess@@YAHHHH@Z DD imagerel $LN10
	DD	imagerel $LN10+258
	DD	imagerel $unwind$?SetFileToProcess@@YAHHHH@Z
$pdata$?SendSignal@@YAHHH@Z DD imagerel $LN5
	DD	imagerel $LN5+103
	DD	imagerel $unwind$?SendSignal@@YAHHH@Z
$pdata$?GetCurrentTime@@YAHPEAX@Z DD imagerel $LN3
	DD	imagerel $LN3+34
	DD	imagerel $unwind$?GetCurrentTime@@YAHPEAX@Z
pdata	ENDS
xdata	SEGMENT
$unwind$?PauseThread@@YAHXZ DD 010401H
	DD	06204H
$unwind$?GetThreadID@@YAGXZ DD 010401H
	DD	06204H
$unwind$?GetProcessID@@YAHXZ DD 010401H
	DD	06204H
$unwind$?ProcessExit@@YAHXZ DD 010401H
	DD	06204H
$unwind$?ProcessWaitForTermination@@YAHH@Z DD 010801H
	DD	06208H
$unwind$?CreateProcess@@YAHHPEAD@Z DD 010d01H
	DD	0620dH
$unwind$?ProcessLoadExec@@YAHHPEADHPEAPEAD@Z DD 011701H
	DD	0a217H
$unwind$?ProcessSleep@@YAH_K@Z DD 010901H
	DD	06209H
$unwind$?SignalReturn@@YAXH@Z DD 010801H
	DD	04208H
$unwind$?SetSignal@@YAHHP6AXH@Z@Z DD 010d01H
	DD	0620dH
$unwind$?GetSystemTimerTick@@YA_KXZ DD 010401H
	DD	04204H
$unwind$?CreateUserThread@@YAHP6AXPEAX@ZPEAD@Z DD 010e01H
	DD	0820eH
$unwind$?CloseUserThread@@YAHH@Z DD 010801H
	DD	06208H
$unwind$?SetFileToProcess@@YAHHHH@Z DD 011101H
	DD	0a211H
$unwind$?SendSignal@@YAHHH@Z DD 010c01H
	DD	0620cH
$unwind$?GetCurrentTime@@YAHPEAX@Z DD 010901H
	DD	04209H
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
ptr$ = 48
?GetCurrentTime@@YAHPEAX@Z PROC				; GetCurrentTime

; 316  : int GetCurrentTime(void* ptr) {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 317  : 	x64_cli();

	call	x64_cli

; 318  : 	AuGetCurrentTime((AuTime*)ptr);

	mov	rcx, QWORD PTR ptr$[rsp]
	call	AuGetCurrentTime

; 319  : 	return 1;

	mov	eax, 1

; 320  : }

	add	rsp, 40					; 00000028H
	ret	0
?GetCurrentTime@@YAHPEAX@Z ENDP				; GetCurrentTime
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
proc$ = 32
mainthr$ = 40
pid$ = 64
signo$ = 72
?SendSignal@@YAHHH@Z PROC				; SendSignal

; 299  : int SendSignal(int pid,int signo) {

$LN5:
	mov	DWORD PTR [rsp+16], edx
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 56					; 00000038H

; 300  : 	x64_cli();

	call	x64_cli

; 301  : 	AuProcess* proc = AuProcessFindPID(pid);

	mov	ecx, DWORD PTR pid$[rsp]
	call	?AuProcessFindPID@@YAPEAU_au_proc_@@H@Z	; AuProcessFindPID
	mov	QWORD PTR proc$[rsp], rax

; 302  : 	if (!proc)

	cmp	QWORD PTR proc$[rsp], 0
	jne	SHORT $LN2@SendSignal

; 303  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN3@SendSignal
$LN2@SendSignal:

; 304  : 
; 305  : 	AuThread* mainthr = proc->main_thread;

	mov	rax, QWORD PTR proc$[rsp]
	mov	rax, QWORD PTR [rax+72]
	mov	QWORD PTR mainthr$[rsp], rax

; 306  : 	if (!mainthr)

	cmp	QWORD PTR mainthr$[rsp], 0
	jne	SHORT $LN1@SendSignal

; 307  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN3@SendSignal
$LN1@SendSignal:

; 308  : 	AuSendSignal(mainthr->id, signo);

	mov	edx, DWORD PTR signo$[rsp]
	mov	rax, QWORD PTR mainthr$[rsp]
	movzx	ecx, WORD PTR [rax+301]
	call	?AuSendSignal@@YAXGH@Z			; AuSendSignal

; 309  : 	return 0;

	xor	eax, eax
$LN3@SendSignal:

; 310  : }

	add	rsp, 56					; 00000038H
	ret	0
?SendSignal@@YAHHH@Z ENDP				; SendSignal
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
proc$ = 32
file$ = 40
thr$ = 48
destproc$ = 56
destfile$ = 64
fileno$ = 96
dest_fdidx$ = 104
proc_id$ = 112
?SetFileToProcess@@YAHHHH@Z PROC			; SetFileToProcess

; 247  : int SetFileToProcess(int fileno, int dest_fdidx, int proc_id) {

$LN10:
	mov	DWORD PTR [rsp+24], r8d
	mov	DWORD PTR [rsp+16], edx
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 88					; 00000058H

; 248  : 	x64_cli();

	call	x64_cli

; 249  : 	AuThread* thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR thr$[rsp], rax

; 250  : 	if (!thr)

	cmp	QWORD PTR thr$[rsp], 0
	jne	SHORT $LN7@SetFileToP

; 251  : 		return 0;

	xor	eax, eax
	jmp	$LN8@SetFileToP
$LN7@SetFileToP:

; 252  : 	/* file check if current thread's process is
; 253  : 	 * found by checking twice, first by
; 254  : 	 * main thread checkup second by sub thread
; 255  : 	 * checkup
; 256  : 	 */
; 257  : 	AuProcess* proc = AuProcessFindThread(thr);

	mov	rcx, QWORD PTR thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR proc$[rsp], rax

; 258  : 	if (!proc) {

	cmp	QWORD PTR proc$[rsp], 0
	jne	SHORT $LN6@SetFileToP

; 259  : 		proc = AuProcessFindSubThread(thr);

	mov	rcx, QWORD PTR thr$[rsp]
	call	?AuProcessFindSubThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindSubThread
	mov	QWORD PTR proc$[rsp], rax

; 260  : 		if (!proc)

	cmp	QWORD PTR proc$[rsp], 0
	jne	SHORT $LN5@SetFileToP

; 261  : 			return -1;

	mov	eax, -1
	jmp	$LN8@SetFileToP
$LN5@SetFileToP:
$LN6@SetFileToP:

; 262  : 	}
; 263  : 
; 264  : 	/* now try getting the destination process by its
; 265  : 	* process id
; 266  : 	*/
; 267  : 	AuProcess* destproc = AuProcessFindPID(proc_id);

	mov	ecx, DWORD PTR proc_id$[rsp]
	call	?AuProcessFindPID@@YAPEAU_au_proc_@@H@Z	; AuProcessFindPID
	mov	QWORD PTR destproc$[rsp], rax

; 268  : 	if (!destproc)

	cmp	QWORD PTR destproc$[rsp], 0
	jne	SHORT $LN4@SetFileToP

; 269  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN8@SetFileToP
$LN4@SetFileToP:

; 270  : 
; 271  : 	/* now try getting the file from current process
; 272  : 	 * file entry
; 273  : 	 */
; 274  : 	AuVFSNode* file = proc->fds[fileno];

	movsxd	rax, DWORD PTR fileno$[rsp]
	mov	rcx, QWORD PTR proc$[rsp]
	mov	rax, QWORD PTR [rcx+rax*8+576]
	mov	QWORD PTR file$[rsp], rax

; 275  : 	if (!file)

	cmp	QWORD PTR file$[rsp], 0
	jne	SHORT $LN3@SetFileToP

; 276  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN8@SetFileToP
$LN3@SetFileToP:

; 277  : 
; 278  : 	AuVFSNode *destfile = destproc->fds[dest_fdidx];

	movsxd	rax, DWORD PTR dest_fdidx$[rsp]
	mov	rcx, QWORD PTR destproc$[rsp]
	mov	rax, QWORD PTR [rcx+rax*8+576]
	mov	QWORD PTR destfile$[rsp], rax

; 279  : 	if (destfile)

	cmp	QWORD PTR destfile$[rsp], 0
	je	SHORT $LN2@SetFileToP

; 280  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN8@SetFileToP

; 281  : 	else {

	jmp	SHORT $LN1@SetFileToP
$LN2@SetFileToP:

; 282  : 		/* now we have no file entry in destination
; 283  : 		 * process's file index, so make entry
; 284  : 		 * of current process's file targeted by
; 285  : 		 * fileno to destination processes file
; 286  : 		 * entry 
; 287  : 		 */
; 288  : 		destproc->fds[dest_fdidx] = file;

	movsxd	rax, DWORD PTR dest_fdidx$[rsp]
	mov	rcx, QWORD PTR destproc$[rsp]
	mov	rdx, QWORD PTR file$[rsp]
	mov	QWORD PTR [rcx+rax*8+576], rdx

; 289  : 		file->fileCopyCount += 1;

	mov	rax, QWORD PTR file$[rsp]
	movzx	eax, WORD PTR [rax+80]
	inc	eax
	mov	rcx, QWORD PTR file$[rsp]
	mov	WORD PTR [rcx+80], ax
$LN1@SetFileToP:
$LN8@SetFileToP:

; 290  : 	}
; 291  : }

	add	rsp, 88					; 00000058H
	ret	0
?SetFileToProcess@@YAHHHH@Z ENDP			; SetFileToProcess
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
thr$ = 32
proc$ = 40
thread_idx$ = 64
?CloseUserThread@@YAHH@Z PROC				; CloseUserThread

; 229  : int CloseUserThread(int thread_idx) {

$LN4:
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 56					; 00000038H

; 230  : 	x64_cli();

	call	x64_cli

; 231  : 	AuThread* thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR thr$[rsp], rax

; 232  : 	if (!thr)

	cmp	QWORD PTR thr$[rsp], 0
	jne	SHORT $LN1@CloseUserT

; 233  : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN2@CloseUserT
$LN1@CloseUserT:

; 234  : 	AuProcess* proc = AuProcessFindThread(thr);

	mov	rcx, QWORD PTR thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR proc$[rsp], rax

; 235  : 
; 236  : 	/* under development*/
; 237  : 	return 0; //NOT IMPLEMENTED

	xor	eax, eax
$LN2@CloseUserT:

; 238  : }

	add	rsp, 56					; 00000038H
	ret	0
?CloseUserThread@@YAHH@Z ENDP				; CloseUserThread
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
idx$ = 32
proc$ = 40
thr$ = 48
entry$ = 80
name$ = 88
?CreateUserThread@@YAHP6AXPEAX@ZPEAD@Z PROC		; CreateUserThread

; 206  : int CreateUserThread(void(*entry) (void*), char *name){

$LN6:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 72					; 00000048H

; 207  : 	x64_cli();

	call	x64_cli

; 208  : 	AuThread* thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR thr$[rsp], rax

; 209  : 	if (!thr)

	cmp	QWORD PTR thr$[rsp], 0
	jne	SHORT $LN3@CreateUser

; 210  : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN4@CreateUser
$LN3@CreateUser:

; 211  : 	AuProcess* proc = AuProcessFindThread(thr);

	mov	rcx, QWORD PTR thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR proc$[rsp], rax

; 212  : 	if (!proc) {

	cmp	QWORD PTR proc$[rsp], 0
	jne	SHORT $LN2@CreateUser

; 213  : 		proc = AuProcessFindSubThread(thr);

	mov	rcx, QWORD PTR thr$[rsp]
	call	?AuProcessFindSubThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindSubThread
	mov	QWORD PTR proc$[rsp], rax

; 214  : 		if (!proc)

	cmp	QWORD PTR proc$[rsp], 0
	jne	SHORT $LN1@CreateUser

; 215  : 			return 0;

	xor	eax, eax
	jmp	SHORT $LN4@CreateUser
$LN1@CreateUser:
$LN2@CreateUser:

; 216  : 	}
; 217  : 	int idx = AuCreateUserthread(proc, entry, name);

	mov	r8, QWORD PTR name$[rsp]
	mov	rdx, QWORD PTR entry$[rsp]
	mov	rcx, QWORD PTR proc$[rsp]
	call	?AuCreateUserthread@@YAHPEAU_au_proc_@@P6AXPEAX@ZPEAD@Z ; AuCreateUserthread
	mov	DWORD PTR idx$[rsp], eax

; 218  : 	return idx;

	mov	eax, DWORD PTR idx$[rsp]
$LN4@CreateUser:

; 219  : }

	add	rsp, 72					; 00000048H
	ret	0
?CreateUserThread@@YAHP6AXPEAX@ZPEAD@Z ENDP		; CreateUserThread
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
?GetSystemTimerTick@@YA_KXZ PROC			; GetSystemTimerTick

; 197  : size_t GetSystemTimerTick() {

$LN3:
	sub	rsp, 40					; 00000028H

; 198  : 	return AuGetSystemTimerTick();

	call	AuGetSystemTimerTick

; 199  : }

	add	rsp, 40					; 00000028H
	ret	0
?GetSystemTimerTick@@YA_KXZ ENDP			; GetSystemTimerTick
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
thr$ = 32
signo$ = 64
handler$ = 72
?SetSignal@@YAHHP6AXH@Z@Z PROC				; SetSignal

; 186  : int SetSignal(int signo, AuSigHandler handler){

$LN4:
	mov	QWORD PTR [rsp+16], rdx
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 56					; 00000038H

; 187  : 	AuThread* thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR thr$[rsp], rax

; 188  : 	if (!thr)

	cmp	QWORD PTR thr$[rsp], 0
	jne	SHORT $LN1@SetSignal

; 189  : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN2@SetSignal
$LN1@SetSignal:

; 190  : 	thr->singals[signo] = handler;

	movsxd	rax, DWORD PTR signo$[rsp]
	mov	rcx, QWORD PTR thr$[rsp]
	mov	rdx, QWORD PTR handler$[rsp]
	mov	QWORD PTR [rcx+rax*8+306], rdx
$LN2@SetSignal:

; 191  : }

	add	rsp, 56					; 00000038H
	ret	0
?SetSignal@@YAHHP6AXH@Z@Z ENDP				; SetSignal
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
num$ = 48
?SignalReturn@@YAXH@Z PROC				; SignalReturn

; 175  : void SignalReturn(int num) {

$LN3:
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 40					; 00000028H

; 176  : 	x64_cli();

	call	x64_cli

; 177  : 	SeTextOut("Signal Return \r\n");

	lea	rcx, OFFSET FLAT:$SG3825
	call	SeTextOut

; 178  : 	/* just make a page fault */
; 179  : }

	add	rsp, 40					; 00000028H
	ret	0
?SignalReturn@@YAXH@Z ENDP				; SignalReturn
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
current_thr$ = 32
sleep_time$ = 40
ms$ = 64
?ProcessSleep@@YAH_K@Z PROC				; ProcessSleep

; 160  : int ProcessSleep(uint64_t ms) {

$LN5:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 161  : 	x64_cli();

	call	x64_cli

; 162  : 	uint64_t sleep_time = ms *100;

	imul	rax, QWORD PTR ms$[rsp], 100		; 00000064H
	mov	QWORD PTR sleep_time$[rsp], rax

; 163  : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 164  : 	if (!current_thr)

	cmp	QWORD PTR current_thr$[rsp], 0
	jne	SHORT $LN2@ProcessSle

; 165  : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN3@ProcessSle
$LN2@ProcessSle:

; 166  : 	if (current_thr->pendingSigCount > 0)

	mov	rax, QWORD PTR current_thr$[rsp]
	movzx	eax, BYTE PTR [rax+626]
	test	eax, eax
	jle	SHORT $LN1@ProcessSle

; 167  : 		return 0;

	xor	eax, eax
	jmp	SHORT $LN3@ProcessSle
$LN1@ProcessSle:

; 168  : 	AuSleepThread(current_thr, sleep_time);

	mov	rdx, QWORD PTR sleep_time$[rsp]
	mov	rcx, QWORD PTR current_thr$[rsp]
	call	AuSleepThread

; 169  : 	AuForceScheduler();

	call	AuForceScheduler
$LN3@ProcessSle:

; 170  : }

	add	rsp, 56					; 00000038H
	ret	0
?ProcessSleep@@YAH_K@Z ENDP				; ProcessSleep
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
char_cnt$ = 32
i$1 = 36
i$2 = 40
status$ = 44
allocated_argv$ = 48
proc$ = 56
l$3 = 64
proc_id$ = 96
filename$ = 104
argc$ = 112
argv$ = 120
?ProcessLoadExec@@YAHHPEADHPEAPEAD@Z PROC		; ProcessLoadExec

; 123  : int ProcessLoadExec(int proc_id, char* filename,int argc, char** argv) {

$LN12:
	mov	QWORD PTR [rsp+32], r9
	mov	DWORD PTR [rsp+24], r8d
	mov	QWORD PTR [rsp+16], rdx
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 88					; 00000058H

; 124  : 	x64_cli();

	call	x64_cli

; 125  : 	AuProcess* proc = AuProcessFindPID(proc_id);

	mov	ecx, DWORD PTR proc_id$[rsp]
	call	?AuProcessFindPID@@YAPEAU_au_proc_@@H@Z	; AuProcessFindPID
	mov	QWORD PTR proc$[rsp], rax

; 126  : 	//SeTextOut("Loading process -> %s \r\n", filename);
; 127  : 	if (!proc) {

	cmp	QWORD PTR proc$[rsp], 0
	jne	SHORT $LN9@ProcessLoa

; 128  : 		AuTextOut("No process found \n");

	lea	rcx, OFFSET FLAT:$SG3798
	call	AuTextOut

; 129  : 		return -1;

	mov	eax, -1
	jmp	$LN10@ProcessLoa
$LN9@ProcessLoa:

; 130  : 	}
; 131  : 	
; 132  : 	/* prepare stuffs for passing arguments */
; 133  : 	int char_cnt = 0;

	mov	DWORD PTR char_cnt$[rsp], 0

; 134  : 	for (int i = 0; i < argc; i++) {

	mov	DWORD PTR i$2[rsp], 0
	jmp	SHORT $LN8@ProcessLoa
$LN7@ProcessLoa:
	mov	eax, DWORD PTR i$2[rsp]
	inc	eax
	mov	DWORD PTR i$2[rsp], eax
$LN8@ProcessLoa:
	mov	eax, DWORD PTR argc$[rsp]
	cmp	DWORD PTR i$2[rsp], eax
	jge	SHORT $LN6@ProcessLoa

; 135  : 		size_t l = strlen(argv[i]);

	movsxd	rax, DWORD PTR i$2[rsp]
	mov	rcx, QWORD PTR argv$[rsp]
	mov	rcx, QWORD PTR [rcx+rax*8]
	call	strlen
	mov	QWORD PTR l$3[rsp], rax

; 136  : 		char_cnt += l; 

	movsxd	rax, DWORD PTR char_cnt$[rsp]
	add	rax, QWORD PTR l$3[rsp]
	mov	DWORD PTR char_cnt$[rsp], eax

; 137  : 	}

	jmp	SHORT $LN7@ProcessLoa
$LN6@ProcessLoa:

; 138  : 	
; 139  : 	char** allocated_argv = 0;

	mov	QWORD PTR allocated_argv$[rsp], 0

; 140  : 	if (char_cnt > 0) {

	cmp	DWORD PTR char_cnt$[rsp], 0
	jle	SHORT $LN5@ProcessLoa

; 141  : 		allocated_argv = (char**)kmalloc(char_cnt);

	mov	ecx, DWORD PTR char_cnt$[rsp]
	call	kmalloc
	mov	QWORD PTR allocated_argv$[rsp], rax

; 142  : 		memset(allocated_argv, 0, char_cnt);

	mov	r8d, DWORD PTR char_cnt$[rsp]
	xor	edx, edx
	mov	rcx, QWORD PTR allocated_argv$[rsp]
	call	memset

; 143  : 		for (int i = 0; i < argc; i++)

	mov	DWORD PTR i$1[rsp], 0
	jmp	SHORT $LN4@ProcessLoa
$LN3@ProcessLoa:
	mov	eax, DWORD PTR i$1[rsp]
	inc	eax
	mov	DWORD PTR i$1[rsp], eax
$LN4@ProcessLoa:
	mov	eax, DWORD PTR argc$[rsp]
	cmp	DWORD PTR i$1[rsp], eax
	jge	SHORT $LN2@ProcessLoa

; 144  : 			allocated_argv[i] = argv[i];

	movsxd	rax, DWORD PTR i$1[rsp]
	movsxd	rcx, DWORD PTR i$1[rsp]
	mov	rdx, QWORD PTR allocated_argv$[rsp]
	mov	r8, QWORD PTR argv$[rsp]
	mov	rax, QWORD PTR [r8+rax*8]
	mov	QWORD PTR [rdx+rcx*8], rax
	jmp	SHORT $LN3@ProcessLoa
$LN2@ProcessLoa:
$LN5@ProcessLoa:

; 145  : 	}
; 146  : 	
; 147  : 	int status = AuLoadExecToProcess(proc, filename, argc,allocated_argv);

	mov	r9, QWORD PTR allocated_argv$[rsp]
	mov	r8d, DWORD PTR argc$[rsp]
	mov	rdx, QWORD PTR filename$[rsp]
	mov	rcx, QWORD PTR proc$[rsp]
	call	?AuLoadExecToProcess@@YAHPEAU_au_proc_@@PEADHPEAPEAD@Z ; AuLoadExecToProcess
	mov	DWORD PTR status$[rsp], eax

; 148  : 	if (status == -1) {

	cmp	DWORD PTR status$[rsp], -1
	jne	SHORT $LN1@ProcessLoa

; 149  : 		SeTextOut("Process launched failed %s\r\n", filename);

	mov	rdx, QWORD PTR filename$[rsp]
	lea	rcx, OFFSET FLAT:$SG3814
	call	SeTextOut

; 150  : 		AuProcessExit(proc, true);

	mov	dl, 1
	mov	rcx, QWORD PTR proc$[rsp]
	call	?AuProcessExit@@YAXPEAU_au_proc_@@_N@Z	; AuProcessExit

; 151  : 		return -1;

	mov	eax, -1
$LN1@ProcessLoa:
$LN10@ProcessLoa:

; 152  : 	}
; 153  : }

	add	rsp, 88					; 00000058H
	ret	0
?ProcessLoadExec@@YAHHPEADHPEAPEAD@Z ENDP		; ProcessLoadExec
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
slot$ = 32
parent_id$ = 64
name$ = 72
?CreateProcess@@YAHHPEAD@Z PROC				; CreateProcess

; 112  : int CreateProcess(int parent_id, char *name) {

$LN4:
	mov	QWORD PTR [rsp+16], rdx
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 56					; 00000038H

; 113  : 	AuProcess* slot = AuCreateProcessSlot(0, name);

	mov	rdx, QWORD PTR name$[rsp]
	xor	ecx, ecx
	call	?AuCreateProcessSlot@@YAPEAU_au_proc_@@PEAU1@PEAD@Z ; AuCreateProcessSlot
	mov	QWORD PTR slot$[rsp], rax

; 114  : 	if (!slot)

	cmp	QWORD PTR slot$[rsp], 0
	jne	SHORT $LN1@CreateProc

; 115  : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN2@CreateProc
$LN1@CreateProc:

; 116  : 	return slot->proc_id;

	mov	rax, QWORD PTR slot$[rsp]
	mov	eax, DWORD PTR [rax]
$LN2@CreateProc:

; 117  : }

	add	rsp, 56					; 00000038H
	ret	0
?CreateProcess@@YAHHPEAD@Z ENDP				; CreateProcess
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
current_thr$ = 32
proc$ = 40
pid$ = 64
?ProcessWaitForTermination@@YAHH@Z PROC			; ProcessWaitForTermination

; 100  : int ProcessWaitForTermination(int pid) {

$LN3:
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 56					; 00000038H

; 101  : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 102  : 	AuProcess* proc = AuProcessFindThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR proc$[rsp], rax

; 103  : 	AuProcessWaitForTermination(proc, pid);

	mov	edx, DWORD PTR pid$[rsp]
	mov	rcx, QWORD PTR proc$[rsp]
	call	?AuProcessWaitForTermination@@YAXPEAU_au_proc_@@H@Z ; AuProcessWaitForTermination

; 104  : 	return 0;

	xor	eax, eax

; 105  : }

	add	rsp, 56					; 00000038H
	ret	0
?ProcessWaitForTermination@@YAHH@Z ENDP			; ProcessWaitForTermination
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
proc$ = 32
current_thr$ = 40
?ProcessExit@@YAHXZ PROC				; ProcessExit

; 86   : int ProcessExit() {

$LN4:
	sub	rsp, 56					; 00000038H

; 87   : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 88   : 	AuProcess* proc = AuProcessFindThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR proc$[rsp], rax

; 89   : 	if (!proc)

	cmp	QWORD PTR proc$[rsp], 0
	jne	SHORT $LN1@ProcessExi

; 90   : 		return 1;

	mov	eax, 1
	jmp	SHORT $LN2@ProcessExi
$LN1@ProcessExi:

; 91   : 	AuProcessExit(proc, true);

	mov	dl, 1
	mov	rcx, QWORD PTR proc$[rsp]
	call	?AuProcessExit@@YAXPEAU_au_proc_@@_N@Z	; AuProcessExit

; 92   : 	return 0;

	xor	eax, eax
$LN2@ProcessExi:

; 93   : }

	add	rsp, 56					; 00000038H
	ret	0
?ProcessExit@@YAHXZ ENDP				; ProcessExit
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
proc$ = 32
current_thr$ = 40
?GetProcessID@@YAHXZ PROC				; GetProcessID

; 68   : int GetProcessID() {

$LN6:
	sub	rsp, 56					; 00000038H

; 69   : 	AuThread * current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 70   : 	if (!current_thr)

	cmp	QWORD PTR current_thr$[rsp], 0
	jne	SHORT $LN3@GetProcess

; 71   : 		return -1;

	mov	eax, -1
	jmp	SHORT $LN4@GetProcess
$LN3@GetProcess:

; 72   : 	AuProcess* proc = AuProcessFindThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR proc$[rsp], rax

; 73   : 	if (!proc){

	cmp	QWORD PTR proc$[rsp], 0
	jne	SHORT $LN2@GetProcess

; 74   : 		proc = AuProcessFindSubThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	?AuProcessFindSubThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindSubThread
	mov	QWORD PTR proc$[rsp], rax

; 75   : 		if (!proc){

	cmp	QWORD PTR proc$[rsp], 0
	jne	SHORT $LN1@GetProcess

; 76   : 			return -1;

	mov	eax, -1
	jmp	SHORT $LN4@GetProcess
$LN1@GetProcess:
$LN2@GetProcess:

; 77   : 		}
; 78   : 	}
; 79   : 	return proc->proc_id;

	mov	rax, QWORD PTR proc$[rsp]
	mov	eax, DWORD PTR [rax]
$LN4@GetProcess:

; 80   : }

	add	rsp, 56					; 00000038H
	ret	0
?GetProcessID@@YAHXZ ENDP				; GetProcessID
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
current_thr$ = 32
?GetThreadID@@YAGXZ PROC				; GetThreadID

; 57   : uint16_t GetThreadID() {

$LN4:
	sub	rsp, 56					; 00000038H

; 58   : 	AuThread* current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 59   : 	if (!current_thr)

	cmp	QWORD PTR current_thr$[rsp], 0
	jne	SHORT $LN1@GetThreadI

; 60   : 		return -1;

	mov	eax, 65535				; 0000ffffH
	jmp	SHORT $LN2@GetThreadI
$LN1@GetThreadI:

; 61   : 	return current_thr->id;

	mov	rax, QWORD PTR current_thr$[rsp]
	movzx	eax, WORD PTR [rax+301]
$LN2@GetThreadI:

; 62   : }

	add	rsp, 56					; 00000038H
	ret	0
?GetThreadID@@YAGXZ ENDP				; GetThreadID
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\serv\thrserv.cpp
_TEXT	SEGMENT
current_thr$ = 32
?PauseThread@@YAHXZ PROC				; PauseThread

; 46   : int PauseThread() {

$LN3:
	sub	rsp, 56					; 00000038H

; 47   : 	x64_cli();

	call	x64_cli

; 48   : 	AuThread * current_thr = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR current_thr$[rsp], rax

; 49   : 	AuBlockThread(current_thr);

	mov	rcx, QWORD PTR current_thr$[rsp]
	call	AuBlockThread

; 50   : 	x64_force_sched();

	call	x64_force_sched

; 51   : 	return 1;

	mov	eax, 1

; 52   : }

	add	rsp, 56					; 00000038H
	ret	0
?PauseThread@@YAHXZ ENDP				; PauseThread
_TEXT	ENDS
END
