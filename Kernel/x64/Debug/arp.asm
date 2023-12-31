; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

CONST	SEGMENT
$SG2975	DB	'ARP Setuped ', 0aH, 00H
CONST	ENDS
PUBLIC	?AuARPRequestMAC@@YAXXZ				; AuARPRequestMAC
EXTRN	?AuEthernetSend@@YAXPEAX_KGPEAE@Z:PROC		; AuEthernetSend
EXTRN	AuGetNetworkAdapterByType:PROC
EXTRN	memset:PROC
EXTRN	memcpy:PROC
EXTRN	AuTextOut:PROC
pdata	SEGMENT
$pdata$?AuARPRequestMAC@@YAXXZ DD imagerel $LN4
	DD	imagerel $LN4+234
	DD	imagerel $unwind$?AuARPRequestMAC@@YAXXZ
pdata	ENDS
xdata	SEGMENT
$unwind$?AuARPRequestMAC@@YAXXZ DD 010401H
	DD	0a204H
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\net\arp.cpp
_TEXT	SEGMENT
netadapt$ = 32
mac$ = 40
arp$ = 48
?AuARPRequestMAC@@YAXXZ PROC				; AuARPRequestMAC

; 40   : void AuARPRequestMAC() {

$LN4:
	sub	rsp, 88					; 00000058H

; 41   : 	AuNetAdapter* netadapt = AuGetNetworkAdapterByType(AUNET_HWTYPE_ETHERNET);

	mov	cl, 1
	call	AuGetNetworkAdapterByType
	mov	QWORD PTR netadapt$[rsp], rax

; 42   : 	if (!netadapt)

	cmp	QWORD PTR netadapt$[rsp], 0
	jne	SHORT $LN1@AuARPReque

; 43   : 		return;

	jmp	$LN2@AuARPReque
$LN1@AuARPReque:

; 44   : 
; 45   : 	NetARP arp;
; 46   : 	arp.hwAddressType = 0x0100;

	mov	eax, 256				; 00000100H
	mov	WORD PTR arp$[rsp], ax

; 47   : 	arp.hwProtocolType = 0x0008;

	mov	eax, 8
	mov	WORD PTR arp$[rsp+2], ax

; 48   : 	arp.hwAddressSize = 6;

	mov	BYTE PTR arp$[rsp+4], 6

; 49   : 	arp.protocolSize = 4;

	mov	BYTE PTR arp$[rsp+5], 4

; 50   : 	arp.operation = ARP_OPERATION_REQUEST;

	mov	eax, 256				; 00000100H
	mov	WORD PTR arp$[rsp+6], ax

; 51   : 
; 52   : 	uint8_t* mac = netadapt->mac;

	mov	rax, QWORD PTR netadapt$[rsp]
	mov	QWORD PTR mac$[rsp], rax

; 53   : 	memcpy(arp.srcMac, mac, 6);

	mov	r8d, 6
	mov	rdx, QWORD PTR mac$[rsp]
	lea	rcx, QWORD PTR arp$[rsp+8]
	call	memcpy

; 54   : 	arp.srcIP[0] = 10; //192.168.0.0

	mov	eax, 1
	imul	rax, rax, 0
	mov	BYTE PTR arp$[rsp+rax+14], 10

; 55   : 	arp.srcIP[1] = 0;

	mov	eax, 1
	imul	rax, rax, 1
	mov	BYTE PTR arp$[rsp+rax+14], 0

; 56   : 	arp.srcIP[2] = 2;

	mov	eax, 1
	imul	rax, rax, 2
	mov	BYTE PTR arp$[rsp+rax+14], 2

; 57   : 	arp.srcIP[3] = 14;

	mov	eax, 1
	imul	rax, rax, 3
	mov	BYTE PTR arp$[rsp+rax+14], 14

; 58   : 
; 59   : 	memset(arp.destMac, 0xff, 6);

	mov	r8d, 6
	mov	dl, 255					; 000000ffH
	lea	rcx, QWORD PTR arp$[rsp+18]
	call	memset

; 60   : 	memset(arp.destIP, 0xff, 4);

	mov	r8d, 4
	mov	dl, 255					; 000000ffH
	lea	rcx, QWORD PTR arp$[rsp+24]
	call	memset

; 61   : 	AuTextOut("ARP Setuped \n");

	lea	rcx, OFFSET FLAT:$SG2975
	call	AuTextOut

; 62   : 	
; 63   : 	AuEthernetSend(&arp, sizeof(NetARP), ETHERNET_TYPE_ARP, arp.destMac);

	lea	r9, QWORD PTR arp$[rsp+18]
	mov	r8w, 2054				; 00000806H
	mov	edx, 28
	lea	rcx, QWORD PTR arp$[rsp]
	call	?AuEthernetSend@@YAXPEAX_KGPEAE@Z	; AuEthernetSend
$LN2@AuARPReque:

; 64   : }

	add	rsp, 88					; 00000058H
	ret	0
?AuARPRequestMAC@@YAXXZ ENDP				; AuARPRequestMAC
_TEXT	ENDS
END
