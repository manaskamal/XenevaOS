; Listing generated by Microsoft (R) Optimizing Compiler Version 19.29.30154.0 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

msvcjmc	SEGMENT
__8690E104_tcp@cpp DB 01H
msvcjmc	ENDS
PUBLIC	?CreateTCPSocket@@YAHXZ				; CreateTCPSocket
PUBLIC	?CalculateTCPChecksum@@YAGPEAU_tcpcheckheader_@@PEAU_tcphead_@@PEAX_K@Z ; CalculateTCPChecksum
PUBLIC	?AuTCPReceive@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z ; AuTCPReceive
PUBLIC	?AuTCPSend@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z	; AuTCPSend
PUBLIC	?AuTCPClose@@YAXPEAU_socket_@@@Z		; AuTCPClose
PUBLIC	?AuTCPConnect@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z ; AuTCPConnect
PUBLIC	?AuTCPBind@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z ; AuTCPBind
PUBLIC	?AuTCPRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z	; AuTCPRead
PUBLIC	?AuTCPWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z	; AuTCPWrite
PUBLIC	__JustMyCode_Default
PUBLIC	??_C@_0BG@HGJDMLEC@TCP?5Socket?5created?5?$AN?6@ ; `string'
EXTRN	kmalloc:PROC
EXTRN	memset:PROC
EXTRN	AuGetCurrentThread:PROC
EXTRN	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z:PROC ; AuProcessFindThread
EXTRN	?AuProcessFindSubThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z:PROC ; AuProcessFindSubThread
EXTRN	?AuProcessGetFileDesc@@YAHPEAU_au_proc_@@@Z:PROC ; AuProcessGetFileDesc
EXTRN	SeTextOut:PROC
EXTRN	__CheckForDebuggerJustMyCode:PROC
;	COMDAT pdata
pdata	SEGMENT
$pdata$?CreateTCPSocket@@YAHXZ DD imagerel $LN6
	DD	imagerel $LN6+268
	DD	imagerel $unwind$?CreateTCPSocket@@YAHXZ
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?CalculateTCPChecksum@@YAGPEAU_tcpcheckheader_@@PEAU_tcphead_@@PEAX_K@Z DD imagerel $LN17
	DD	imagerel $LN17+623
	DD	imagerel $unwind$?CalculateTCPChecksum@@YAGPEAU_tcpcheckheader_@@PEAU_tcphead_@@PEAX_K@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?AuTCPReceive@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z DD imagerel $LN3
	DD	imagerel $LN3+45
	DD	imagerel $unwind$?AuTCPReceive@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?AuTCPSend@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z DD imagerel $LN3
	DD	imagerel $LN3+45
	DD	imagerel $unwind$?AuTCPSend@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?AuTCPClose@@YAXPEAU_socket_@@@Z DD imagerel $LN3
	DD	imagerel $LN3+33
	DD	imagerel $unwind$?AuTCPClose@@YAXPEAU_socket_@@@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?AuTCPConnect@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z DD imagerel $LN3
	DD	imagerel $LN3+45
	DD	imagerel $unwind$?AuTCPConnect@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?AuTCPBind@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z DD imagerel $LN3
	DD	imagerel $LN3+45
	DD	imagerel $unwind$?AuTCPBind@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?AuTCPRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD imagerel $LN3
	DD	imagerel $LN3+50
	DD	imagerel $unwind$?AuTCPRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?AuTCPWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD imagerel $LN3
	DD	imagerel $LN3+50
	DD	imagerel $unwind$?AuTCPWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z
pdata	ENDS
;	COMDAT ??_C@_0BG@HGJDMLEC@TCP?5Socket?5created?5?$AN?6@
CONST	SEGMENT
??_C@_0BG@HGJDMLEC@TCP?5Socket?5created?5?$AN?6@ DB 'TCP Socket created ', 0dH
	DB	0aH, 00H					; `string'
CONST	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?AuTCPWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD 025031e01H
	DD	0b219231eH
	DD	05015H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?AuTCPRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z DD 025031e01H
	DD	0b219231eH
	DD	05015H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?AuTCPBind@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z DD 025031901H
	DD	0b2142319H
	DD	05010H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?AuTCPConnect@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z DD 025031901H
	DD	0b2142319H
	DD	05010H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?AuTCPClose@@YAXPEAU_socket_@@@Z DD 025030f01H
	DD	0b20a230fH
	DD	05006H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?AuTCPSend@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z DD 025031901H
	DD	0b2142319H
	DD	05010H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?AuTCPReceive@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z DD 025031901H
	DD	0b2142319H
	DD	05010H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?CalculateTCPChecksum@@YAGPEAU_tcpcheckheader_@@PEAU_tcphead_@@PEAX_K@Z DD 025042101H
	DD	011c2321H
	DD	050150014H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?CreateTCPSocket@@YAHXZ DD 025030e01H
	DD	0f209230eH
	DD	05002H
xdata	ENDS
; Function compile flags: /Odt
;	COMDAT __JustMyCode_Default
_TEXT	SEGMENT
__JustMyCode_Default PROC				; COMDAT
	ret	0
__JustMyCode_Default ENDP
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Net\tcp.cpp
;	COMDAT ?AuTCPWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z
_TEXT	SEGMENT
node$ = 80
file$ = 88
buffer$ = 96
len$ = 104
?AuTCPWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z PROC	; AuTCPWrite, COMDAT

; 157  : uint64_t AuTCPWrite(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t len) {

$LN3:
	mov	DWORD PTR [rsp+32], r9d
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	push	rbp
	sub	rsp, 96					; 00000060H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__8690E104_tcp@cpp
	call	__CheckForDebuggerJustMyCode

; 158  : 	return 0;

	xor	eax, eax

; 159  : }

	lea	rsp, QWORD PTR [rbp+64]
	pop	rbp
	ret	0
?AuTCPWrite@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ENDP	; AuTCPWrite
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Net\tcp.cpp
;	COMDAT ?AuTCPRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z
_TEXT	SEGMENT
node$ = 80
file$ = 88
buffer$ = 96
len$ = 104
?AuTCPRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z PROC	; AuTCPRead, COMDAT

; 153  : uint64_t AuTCPRead(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t len){

$LN3:
	mov	DWORD PTR [rsp+32], r9d
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	push	rbp
	sub	rsp, 96					; 00000060H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__8690E104_tcp@cpp
	call	__CheckForDebuggerJustMyCode

; 154  : 	return 0;

	xor	eax, eax

; 155  : }

	lea	rsp, QWORD PTR [rbp+64]
	pop	rbp
	ret	0
?AuTCPRead@@YA_KPEAU__VFS_NODE__@@0PEA_KI@Z ENDP	; AuTCPRead
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Net\tcp.cpp
;	COMDAT ?AuTCPBind@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z
_TEXT	SEGMENT
sock$ = 80
addr$ = 88
addrlen$ = 96
?AuTCPBind@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z PROC	; AuTCPBind, COMDAT

; 148  : int AuTCPBind(AuSocket* sock, sockaddr* addr, socklen_t addrlen){

$LN3:
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	push	rbp
	sub	rsp, 96					; 00000060H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__8690E104_tcp@cpp
	call	__CheckForDebuggerJustMyCode

; 149  : 	return 0;

	xor	eax, eax

; 150  : }

	lea	rsp, QWORD PTR [rbp+64]
	pop	rbp
	ret	0
?AuTCPBind@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z ENDP	; AuTCPBind
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Net\tcp.cpp
;	COMDAT ?AuTCPConnect@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z
_TEXT	SEGMENT
sock$ = 80
addr$ = 88
addrlen$ = 96
?AuTCPConnect@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z PROC ; AuTCPConnect, COMDAT

; 143  : int AuTCPConnect(AuSocket* sock, sockaddr* addr, socklen_t addrlen){

$LN3:
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	push	rbp
	sub	rsp, 96					; 00000060H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__8690E104_tcp@cpp
	call	__CheckForDebuggerJustMyCode

; 144  : 	return 0;

	xor	eax, eax

; 145  : }

	lea	rsp, QWORD PTR [rbp+64]
	pop	rbp
	ret	0
?AuTCPConnect@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z ENDP ; AuTCPConnect
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Net\tcp.cpp
;	COMDAT ?AuTCPClose@@YAXPEAU_socket_@@@Z
_TEXT	SEGMENT
sock$ = 80
?AuTCPClose@@YAXPEAU_socket_@@@Z PROC			; AuTCPClose, COMDAT

; 133  : void AuTCPClose(AuSocket* sock) {

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	push	rbp
	sub	rsp, 96					; 00000060H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__8690E104_tcp@cpp
	call	__CheckForDebuggerJustMyCode

; 134  : 	return;
; 135  : }

	lea	rsp, QWORD PTR [rbp+64]
	pop	rbp
	ret	0
?AuTCPClose@@YAXPEAU_socket_@@@Z ENDP			; AuTCPClose
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Net\tcp.cpp
;	COMDAT ?AuTCPSend@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z
_TEXT	SEGMENT
sock$ = 80
msg$ = 88
flags$ = 96
?AuTCPSend@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z PROC	; AuTCPSend, COMDAT

; 125  : int AuTCPSend(AuSocket* sock, msghdr* msg, int flags){

$LN3:
	mov	DWORD PTR [rsp+24], r8d
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	push	rbp
	sub	rsp, 96					; 00000060H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__8690E104_tcp@cpp
	call	__CheckForDebuggerJustMyCode

; 126  : 	return 0;

	xor	eax, eax

; 127  : }

	lea	rsp, QWORD PTR [rbp+64]
	pop	rbp
	ret	0
?AuTCPSend@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z ENDP	; AuTCPSend
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Net\tcp.cpp
;	COMDAT ?AuTCPReceive@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z
_TEXT	SEGMENT
sock$ = 80
msg$ = 88
flags$ = 96
?AuTCPReceive@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z PROC	; AuTCPReceive, COMDAT

; 115  : int AuTCPReceive(AuSocket* sock, msghdr *msg, int flags){

$LN3:
	mov	DWORD PTR [rsp+24], r8d
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	push	rbp
	sub	rsp, 96					; 00000060H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__8690E104_tcp@cpp
	call	__CheckForDebuggerJustMyCode

; 116  : 	return 0;

	xor	eax, eax

; 117  : }

	lea	rsp, QWORD PTR [rbp+64]
	pop	rbp
	ret	0
?AuTCPReceive@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z ENDP	; AuTCPReceive
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Net\tcp.cpp
;	COMDAT ?CalculateTCPChecksum@@YAGPEAU_tcpcheckheader_@@PEAU_tcphead_@@PEAX_K@Z
_TEXT	SEGMENT
sum$ = 0
s$ = 8
i$1 = 16
i$2 = 20
dwords$ = 24
i$3 = 28
t$4 = 32
tmp$5 = 40
f$6 = 48
p$ = 144
h$ = 152
d$ = 160
payloadsz$ = 168
?CalculateTCPChecksum@@YAGPEAU_tcpcheckheader_@@PEAU_tcphead_@@PEAX_K@Z PROC ; CalculateTCPChecksum, COMDAT

; 69   : uint16_t CalculateTCPChecksum(TCPCheckHeader* p, TCPHeader* h, void* d, size_t payloadsz) {

$LN17:
	mov	QWORD PTR [rsp+32], r9
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	push	rbp
	sub	rsp, 160				; 000000a0H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__8690E104_tcp@cpp
	call	__CheckForDebuggerJustMyCode

; 70   : 	uint32_t sum = 0;

	mov	DWORD PTR sum$[rbp], 0

; 71   : 	uint16_t *s = (uint16_t*)p;

	mov	rax, QWORD PTR p$[rbp]
	mov	QWORD PTR s$[rbp], rax

; 72   : 
; 73   : 	for (int i = 0; i < 6; ++i) {

	mov	DWORD PTR i$1[rbp], 0
	jmp	SHORT $LN4@CalculateT
$LN2@CalculateT:
	mov	eax, DWORD PTR i$1[rbp]
	inc	eax
	mov	DWORD PTR i$1[rbp], eax
$LN4@CalculateT:
	cmp	DWORD PTR i$1[rbp], 6
	jge	SHORT $LN3@CalculateT

; 74   : 		sum += ntohs(s[i]);

	movsxd	rax, DWORD PTR i$1[rbp]
	mov	rcx, QWORD PTR s$[rbp]
	movzx	eax, WORD PTR [rcx+rax*2]
	and	eax, 255				; 000000ffH
	shl	eax, 8
	movsxd	rcx, DWORD PTR i$1[rbp]
	mov	rdx, QWORD PTR s$[rbp]
	movzx	ecx, WORD PTR [rdx+rcx*2]
	and	ecx, 65280				; 0000ff00H
	sar	ecx, 8
	or	eax, ecx
	mov	ecx, DWORD PTR sum$[rbp]
	add	ecx, eax
	mov	eax, ecx
	mov	DWORD PTR sum$[rbp], eax

; 75   : 		if (sum > 0xFFFF)

	cmp	DWORD PTR sum$[rbp], 65535		; 0000ffffH
	jbe	SHORT $LN11@CalculateT

; 76   : 			sum = (sum >> 16) + (sum & 0xFFFF);

	mov	eax, DWORD PTR sum$[rbp]
	shr	eax, 16
	mov	ecx, DWORD PTR sum$[rbp]
	and	ecx, 65535				; 0000ffffH
	add	eax, ecx
	mov	DWORD PTR sum$[rbp], eax
$LN11@CalculateT:

; 77   : 	}

	jmp	SHORT $LN2@CalculateT
$LN3@CalculateT:

; 78   : 
; 79   : 	s = (uint16_t*)h;

	mov	rax, QWORD PTR h$[rbp]
	mov	QWORD PTR s$[rbp], rax

; 80   : 	for (int i = 0; i < 10; ++i) {

	mov	DWORD PTR i$2[rbp], 0
	jmp	SHORT $LN7@CalculateT
$LN5@CalculateT:
	mov	eax, DWORD PTR i$2[rbp]
	inc	eax
	mov	DWORD PTR i$2[rbp], eax
$LN7@CalculateT:
	cmp	DWORD PTR i$2[rbp], 10
	jge	SHORT $LN6@CalculateT

; 81   : 		sum += ntohs(s[i]);

	movsxd	rax, DWORD PTR i$2[rbp]
	mov	rcx, QWORD PTR s$[rbp]
	movzx	eax, WORD PTR [rcx+rax*2]
	and	eax, 255				; 000000ffH
	shl	eax, 8
	movsxd	rcx, DWORD PTR i$2[rbp]
	mov	rdx, QWORD PTR s$[rbp]
	movzx	ecx, WORD PTR [rdx+rcx*2]
	and	ecx, 65280				; 0000ff00H
	sar	ecx, 8
	or	eax, ecx
	mov	ecx, DWORD PTR sum$[rbp]
	add	ecx, eax
	mov	eax, ecx
	mov	DWORD PTR sum$[rbp], eax

; 82   : 		if (sum > 0xFFFF) 

	cmp	DWORD PTR sum$[rbp], 65535		; 0000ffffH
	jbe	SHORT $LN12@CalculateT

; 83   : 			sum = (sum >> 16) + (sum & 0xFFFF);

	mov	eax, DWORD PTR sum$[rbp]
	shr	eax, 16
	mov	ecx, DWORD PTR sum$[rbp]
	and	ecx, 65535				; 0000ffffH
	add	eax, ecx
	mov	DWORD PTR sum$[rbp], eax
$LN12@CalculateT:

; 84   : 	}

	jmp	SHORT $LN5@CalculateT
$LN6@CalculateT:

; 85   : 
; 86   : 	uint16_t dwords = payloadsz / 2;

	xor	edx, edx
	mov	rax, QWORD PTR payloadsz$[rbp]
	mov	ecx, 2
	div	rcx
	mov	WORD PTR dwords$[rbp], ax

; 87   : 	s = (uint16_t*)d;

	mov	rax, QWORD PTR d$[rbp]
	mov	QWORD PTR s$[rbp], rax

; 88   : 	for (unsigned int i = 0; i < dwords; ++i) {

	mov	DWORD PTR i$3[rbp], 0
	jmp	SHORT $LN10@CalculateT
$LN8@CalculateT:
	mov	eax, DWORD PTR i$3[rbp]
	inc	eax
	mov	DWORD PTR i$3[rbp], eax
$LN10@CalculateT:
	movzx	eax, WORD PTR dwords$[rbp]
	cmp	DWORD PTR i$3[rbp], eax
	jae	SHORT $LN9@CalculateT

; 89   : 		sum += ntohs(s[i]);

	mov	eax, DWORD PTR i$3[rbp]
	mov	rcx, QWORD PTR s$[rbp]
	movzx	eax, WORD PTR [rcx+rax*2]
	and	eax, 255				; 000000ffH
	shl	eax, 8
	mov	ecx, DWORD PTR i$3[rbp]
	mov	rdx, QWORD PTR s$[rbp]
	movzx	ecx, WORD PTR [rdx+rcx*2]
	and	ecx, 65280				; 0000ff00H
	sar	ecx, 8
	or	eax, ecx
	mov	ecx, DWORD PTR sum$[rbp]
	add	ecx, eax
	mov	eax, ecx
	mov	DWORD PTR sum$[rbp], eax

; 90   : 		if (sum > 0xFFFF) 

	cmp	DWORD PTR sum$[rbp], 65535		; 0000ffffH
	jbe	SHORT $LN13@CalculateT

; 91   : 			sum = (sum >> 16) + (sum & 0xFFFFF);

	mov	eax, DWORD PTR sum$[rbp]
	shr	eax, 16
	mov	ecx, DWORD PTR sum$[rbp]
	and	ecx, 1048575				; 000fffffH
	add	eax, ecx
	mov	DWORD PTR sum$[rbp], eax
$LN13@CalculateT:

; 92   : 	}

	jmp	SHORT $LN8@CalculateT
$LN9@CalculateT:

; 93   : 
; 94   : 	if (dwords * 2 != payloadsz){

	movzx	eax, WORD PTR dwords$[rbp]
	shl	eax, 1
	cdqe
	cmp	rax, QWORD PTR payloadsz$[rbp]
	je	$LN14@CalculateT

; 95   : 		uint8_t* t = (uint8_t*)d;

	mov	rax, QWORD PTR d$[rbp]
	mov	QWORD PTR t$4[rbp], rax

; 96   : 		uint8_t tmp[2];
; 97   : 		tmp[0] = t[dwords * sizeof(uint16_t)];

	movzx	eax, WORD PTR dwords$[rbp]
	mov	ecx, 1
	imul	rcx, rcx, 0
	mov	rdx, QWORD PTR t$4[rbp]
	movzx	eax, BYTE PTR [rdx+rax*2]
	mov	BYTE PTR tmp$5[rbp+rcx], al

; 98   : 		tmp[1] = 0;

	mov	eax, 1
	imul	rax, rax, 1
	mov	BYTE PTR tmp$5[rbp+rax], 0

; 99   : 
; 100  : 		uint16_t* f = (uint16_t*)tmp;

	lea	rax, QWORD PTR tmp$5[rbp]
	mov	QWORD PTR f$6[rbp], rax

; 101  : 
; 102  : 		sum += ntohs(f[0]);

	mov	eax, 2
	imul	rax, rax, 0
	mov	rcx, QWORD PTR f$6[rbp]
	movzx	eax, WORD PTR [rcx+rax]
	and	eax, 255				; 000000ffH
	shl	eax, 8
	mov	ecx, 2
	imul	rcx, rcx, 0
	mov	rdx, QWORD PTR f$6[rbp]
	movzx	ecx, WORD PTR [rdx+rcx]
	and	ecx, 65280				; 0000ff00H
	sar	ecx, 8
	or	eax, ecx
	mov	ecx, DWORD PTR sum$[rbp]
	add	ecx, eax
	mov	eax, ecx
	mov	DWORD PTR sum$[rbp], eax

; 103  : 		if (sum > 0xFFFF)

	cmp	DWORD PTR sum$[rbp], 65535		; 0000ffffH
	jbe	SHORT $LN15@CalculateT

; 104  : 			sum = (sum >> 16) + (sum & 0xFFFF);

	mov	eax, DWORD PTR sum$[rbp]
	shr	eax, 16
	mov	ecx, DWORD PTR sum$[rbp]
	and	ecx, 65535				; 0000ffffH
	add	eax, ecx
	mov	DWORD PTR sum$[rbp], eax
$LN15@CalculateT:
$LN14@CalculateT:

; 105  : 	}
; 106  : 
; 107  : 	return ~(sum & 0xFFFF) & 0xFFFF;

	mov	eax, DWORD PTR sum$[rbp]
	and	eax, 65535				; 0000ffffH
	not	eax
	and	eax, 65535				; 0000ffffH

; 108  : }

	lea	rsp, QWORD PTR [rbp+128]
	pop	rbp
	ret	0
?CalculateTCPChecksum@@YAGPEAU_tcpcheckheader_@@PEAU_tcphead_@@PEAX_K@Z ENDP ; CalculateTCPChecksum
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Net\tcp.cpp
;	COMDAT ?CreateTCPSocket@@YAHXZ
_TEXT	SEGMENT
fd$ = 0
thread$ = 8
proc$ = 16
sock$ = 24
?CreateTCPSocket@@YAHXZ PROC				; CreateTCPSocket, COMDAT

; 163  : int CreateTCPSocket() {

$LN6:
	push	rbp
	sub	rsp, 128				; 00000080H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__8690E104_tcp@cpp
	call	__CheckForDebuggerJustMyCode

; 164  : 	int fd = -1;

	mov	DWORD PTR fd$[rbp], -1

; 165  : 	AuThread *thread = AuGetCurrentThread();

	call	AuGetCurrentThread
	mov	QWORD PTR thread$[rbp], rax

; 166  : 	if (!thread)

	cmp	QWORD PTR thread$[rbp], 0
	jne	SHORT $LN2@CreateTCPS

; 167  : 		return -1;

	mov	eax, -1
	jmp	$LN1@CreateTCPS
$LN2@CreateTCPS:

; 168  : 	AuProcess *proc = AuProcessFindThread(thread);

	mov	rcx, QWORD PTR thread$[rbp]
	call	?AuProcessFindThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindThread
	mov	QWORD PTR proc$[rbp], rax

; 169  : 	if (!proc) 

	cmp	QWORD PTR proc$[rbp], 0
	jne	SHORT $LN3@CreateTCPS

; 170  : 		proc = AuProcessFindSubThread(thread);

	mov	rcx, QWORD PTR thread$[rbp]
	call	?AuProcessFindSubThread@@YAPEAU_au_proc_@@PEAU_au_thread_@@@Z ; AuProcessFindSubThread
	mov	QWORD PTR proc$[rbp], rax
$LN3@CreateTCPS:

; 171  : 		if (!proc)

	cmp	QWORD PTR proc$[rbp], 0
	jne	SHORT $LN4@CreateTCPS

; 172  : 			return -1;

	mov	eax, -1
	jmp	$LN1@CreateTCPS
$LN4@CreateTCPS:

; 173  : 	AuSocket *sock = (AuSocket*)kmalloc(sizeof(AuSocket));

	mov	ecx, 56					; 00000038H
	call	kmalloc
	mov	QWORD PTR sock$[rbp], rax

; 174  : 	memset(sock, 0, sizeof(AuSocket));

	mov	r8d, 56					; 00000038H
	xor	edx, edx
	mov	rcx, QWORD PTR sock$[rbp]
	call	memset

; 175  : 	//sock->fsnode.flags = FS_FLAG_SOCKET;
; 176  : 	sock->send = AuTCPSend;

	mov	rax, QWORD PTR sock$[rbp]
	lea	rcx, OFFSET FLAT:?AuTCPSend@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z ; AuTCPSend
	mov	QWORD PTR [rax+24], rcx

; 177  : 	sock->receive = AuTCPReceive;

	mov	rax, QWORD PTR sock$[rbp]
	lea	rcx, OFFSET FLAT:?AuTCPReceive@@YAHPEAU_socket_@@PEAU_msghdr_@@H@Z ; AuTCPReceive
	mov	QWORD PTR [rax+16], rcx

; 178  : 	sock->connect = AuTCPConnect;

	mov	rax, QWORD PTR sock$[rbp]
	lea	rcx, OFFSET FLAT:?AuTCPConnect@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z ; AuTCPConnect
	mov	QWORD PTR [rax+40], rcx

; 179  : 	sock->bind = AuTCPBind;

	mov	rax, QWORD PTR sock$[rbp]
	lea	rcx, OFFSET FLAT:?AuTCPBind@@YAHPEAU_socket_@@PEAU_sockaddr_@@_K@Z ; AuTCPBind
	mov	QWORD PTR [rax+48], rcx

; 180  : 	sock->close = AuTCPClose;

	mov	rax, QWORD PTR sock$[rbp]
	lea	rcx, OFFSET FLAT:?AuTCPClose@@YAXPEAU_socket_@@@Z ; AuTCPClose
	mov	QWORD PTR [rax+32], rcx

; 181  : 	/*sock->fsnode.read = AuTCPRead;
; 182  : 	sock->fsnode.write = AuTCPWrite;*/
; 183  : 	fd = AuProcessGetFileDesc(proc);

	mov	rcx, QWORD PTR proc$[rbp]
	call	?AuProcessGetFileDesc@@YAHPEAU_au_proc_@@@Z ; AuProcessGetFileDesc
	mov	DWORD PTR fd$[rbp], eax

; 184  : 	proc->fds[fd] = (AuVFSNode*)sock;

	movsxd	rax, DWORD PTR fd$[rbp]
	mov	rcx, QWORD PTR proc$[rbp]
	mov	rdx, QWORD PTR sock$[rbp]
	mov	QWORD PTR [rcx+rax*8+567], rdx

; 185  : 	SeTextOut("TCP Socket created \r\n");

	lea	rcx, OFFSET FLAT:??_C@_0BG@HGJDMLEC@TCP?5Socket?5created?5?$AN?6@
	call	SeTextOut

; 186  : 	return fd;

	mov	eax, DWORD PTR fd$[rbp]
$LN1@CreateTCPS:

; 187  : }

	lea	rsp, QWORD PTR [rbp+96]
	pop	rbp
	ret	0
?CreateTCPSocket@@YAHXZ ENDP				; CreateTCPSocket
_TEXT	ENDS
END
