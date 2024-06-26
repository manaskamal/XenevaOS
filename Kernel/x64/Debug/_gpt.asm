; Listing generated by Microsoft (R) Optimizing Compiler Version 19.29.30154.0 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

msvcjmc	SEGMENT
__87B18610__gpt@cpp DB 01H
msvcjmc	ENDS
PUBLIC	AuGUIDVerify
PUBLIC	?AuGPTInitialise_FileSystem@@YAXPEAU_VDISK_@@@Z	; AuGPTInitialise_FileSystem
PUBLIC	__JustMyCode_Default
PUBLIC	??_C@_0CM@DFIKACID@GPT?5Partition?5with?5Windows?5Part@ ; `string'
PUBLIC	??_C@_01KMDKNFGN@?1@				; `string'
EXTRN	?FatInitialise@@YAPEAU__VFS_NODE__@@PEAU_VDISK_@@PEAD@Z:PROC ; FatInitialise
EXTRN	AuTextOut:PROC
EXTRN	__CheckForDebuggerJustMyCode:PROC
;	COMDAT pdata
pdata	SEGMENT
$pdata$AuGUIDVerify DD imagerel $LN10
	DD	imagerel $LN10+188
	DD	imagerel $unwind$AuGUIDVerify
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?AuGPTInitialise_FileSystem@@YAXPEAU_VDISK_@@@Z DD imagerel $LN4
	DD	imagerel $LN4+194
	DD	imagerel $unwind$?AuGPTInitialise_FileSystem@@YAXPEAU_VDISK_@@@Z
pdata	ENDS
;	COMDAT ??_C@_01KMDKNFGN@?1@
CONST	SEGMENT
??_C@_01KMDKNFGN@?1@ DB '/', 00H			; `string'
CONST	ENDS
;	COMDAT ??_C@_0CM@DFIKACID@GPT?5Partition?5with?5Windows?5Part@
CONST	SEGMENT
??_C@_0CM@DFIKACID@GPT?5Partition?5with?5Windows?5Part@ DB 'GPT Partition'
	DB	' with Windows Partition data ', 0aH, 00H	; `string'
CONST	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?AuGPTInitialise_FileSystem@@YAXPEAU_VDISK_@@@Z DD 025061401H
	DD	010f2314H
	DD	070080012H
	DD	050066007H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$AuGUIDVerify DD 025031401H
	DD	0d20f2314H
	DD	0500bH
xdata	ENDS
; Function compile flags: /Odt
;	COMDAT __JustMyCode_Default
_TEXT	SEGMENT
__JustMyCode_Default PROC				; COMDAT
	ret	0
__JustMyCode_Default ENDP
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Fs\_gpt.cpp
;	COMDAT ?AuGPTInitialise_FileSystem@@YAXPEAU_VDISK_@@@Z
_TEXT	SEGMENT
guid$ = 0
$T1 = 80
$T2 = 96
vdisk$ = 144
?AuGPTInitialise_FileSystem@@YAXPEAU_VDISK_@@@Z PROC	; AuGPTInitialise_FileSystem, COMDAT

; 62   : void AuGPTInitialise_FileSystem(AuVDisk *vdisk) {

$LN4:
	mov	QWORD PTR [rsp+8], rcx
	push	rbp
	push	rsi
	push	rdi
	sub	rsp, 144				; 00000090H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__87B18610__gpt@cpp
	call	__CheckForDebuggerJustMyCode

; 63   : 	GUID guid = WINDOWS_PARTITION_DATA_GUID;

	mov	DWORD PTR guid$[rbp], -338648926	; ebd0a0a2H
	mov	eax, 47589				; 0000b9e5H
	mov	WORD PTR guid$[rbp+4], ax
	mov	eax, 17459				; 00004433H
	mov	WORD PTR guid$[rbp+6], ax
	mov	BYTE PTR guid$[rbp+8], 135		; 00000087H
	mov	BYTE PTR guid$[rbp+9], 192		; 000000c0H
	mov	BYTE PTR guid$[rbp+10], 104		; 00000068H
	mov	BYTE PTR guid$[rbp+11], 182		; 000000b6H
	mov	BYTE PTR guid$[rbp+12], 183		; 000000b7H
	mov	BYTE PTR guid$[rbp+13], 38		; 00000026H
	mov	BYTE PTR guid$[rbp+14], 153		; 00000099H
	mov	BYTE PTR guid$[rbp+15], 199		; 000000c7H

; 64   : 	
; 65   : 	if (AuGUIDVerify(vdisk->part_guid, guid)) {

	lea	rax, QWORD PTR $T2[rbp]
	lea	rcx, QWORD PTR guid$[rbp]
	mov	rdi, rax
	mov	rsi, rcx
	mov	ecx, 16
	rep movsb
	lea	rax, QWORD PTR $T1[rbp]
	mov	rcx, QWORD PTR vdisk$[rbp]
	mov	rdi, rax
	lea	rsi, QWORD PTR [rcx+102]
	mov	ecx, 16
	rep movsb
	lea	rdx, QWORD PTR $T2[rbp]
	lea	rcx, QWORD PTR $T1[rbp]
	call	AuGUIDVerify
	movzx	eax, al
	test	eax, eax
	je	SHORT $LN2@AuGPTIniti

; 66   : 		AuTextOut("GPT Partition with Windows Partition data \n");

	lea	rcx, OFFSET FLAT:??_C@_0CM@DFIKACID@GPT?5Partition?5with?5Windows?5Part@
	call	AuTextOut

; 67   : 
; 68   : 		/* Now we need to call all Microsoft file systems
; 69   : 		 * detect routine to verify what type of ms file system
; 70   : 		 * it includes, for now we only support FAT file system */
; 71   : 		FatInitialise(vdisk, "/");

	lea	rdx, OFFSET FLAT:??_C@_01KMDKNFGN@?1@
	mov	rcx, QWORD PTR vdisk$[rbp]
	call	?FatInitialise@@YAPEAU__VFS_NODE__@@PEAU_VDISK_@@PEAD@Z ; FatInitialise
$LN2@AuGPTIniti:

; 72   : 	}
; 73   : 
; 74   : 	//guid = LINUX_HOME_PARTITION_GUID;
; 75   : 
; 76   : 	//if (AuGUIDVerify(vdisk->part_guid, guid)) {
; 77   : 	//	AuTextOut("GPT Partition with Linux file systems \n");
; 78   : 	//	/*
; 79   : 	//	 * Now call ext fs detect routine to verify ext version
; 80   : 	//	 * and initialise it
; 81   : 	//	 */
; 82   : 	//}
; 83   : 
; 84   : }

	lea	rsp, QWORD PTR [rbp+112]
	pop	rdi
	pop	rsi
	pop	rbp
	ret	0
?AuGPTInitialise_FileSystem@@YAXPEAU_VDISK_@@@Z ENDP	; AuGPTInitialise_FileSystem
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File E:\Xeneva Project\Aurora\Kernel\Fs\_gpt.cpp
;	COMDAT AuGUIDVerify
_TEXT	SEGMENT
first_part_good$ = 0
i$1 = 4
tv77 = 72
g1$ = 96
g2$ = 104
AuGUIDVerify PROC					; COMDAT

; 43   : bool AuGUIDVerify(GUID g1, GUID g2) {

$LN10:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	push	rbp
	sub	rsp, 112				; 00000070H
	lea	rbp, QWORD PTR [rsp+32]
	lea	rcx, OFFSET FLAT:__87B18610__gpt@cpp
	call	__CheckForDebuggerJustMyCode

; 44   : 	bool first_part_good = (g1.Data1 == g2.Data1 && g1.Data2 == g2.Data2 &&

	mov	rax, QWORD PTR g1$[rbp]
	mov	rcx, QWORD PTR g2$[rbp]
	mov	ecx, DWORD PTR [rcx]
	cmp	DWORD PTR [rax], ecx
	jne	SHORT $LN8@AuGUIDVeri
	mov	rax, QWORD PTR g1$[rbp]
	movzx	eax, WORD PTR [rax+4]
	mov	rcx, QWORD PTR g2$[rbp]
	movzx	ecx, WORD PTR [rcx+4]
	cmp	eax, ecx
	jne	SHORT $LN8@AuGUIDVeri
	mov	rax, QWORD PTR g1$[rbp]
	movzx	eax, WORD PTR [rax+6]
	mov	rcx, QWORD PTR g2$[rbp]
	movzx	ecx, WORD PTR [rcx+6]
	cmp	eax, ecx
	jne	SHORT $LN8@AuGUIDVeri
	mov	DWORD PTR tv77[rbp], 1
	jmp	SHORT $LN9@AuGUIDVeri
$LN8@AuGUIDVeri:
	mov	DWORD PTR tv77[rbp], 0
$LN9@AuGUIDVeri:
	movzx	eax, BYTE PTR tv77[rbp]
	mov	BYTE PTR first_part_good$[rbp], al

; 45   : 		g1.Data3 == g2.Data3);
; 46   : 	if (!first_part_good) return false;

	movzx	eax, BYTE PTR first_part_good$[rbp]
	test	eax, eax
	jne	SHORT $LN5@AuGUIDVeri
	xor	al, al
	jmp	SHORT $LN1@AuGUIDVeri
$LN5@AuGUIDVeri:

; 47   : 
; 48   : 	for (int i = 0; i < 8; ++i){

	mov	DWORD PTR i$1[rbp], 0
	jmp	SHORT $LN4@AuGUIDVeri
$LN2@AuGUIDVeri:
	mov	eax, DWORD PTR i$1[rbp]
	inc	eax
	mov	DWORD PTR i$1[rbp], eax
$LN4@AuGUIDVeri:
	cmp	DWORD PTR i$1[rbp], 8
	jge	SHORT $LN3@AuGUIDVeri

; 49   : 		if (g1.Data4[i] != g2.Data4[i]) 

	movsxd	rax, DWORD PTR i$1[rbp]
	mov	rcx, QWORD PTR g1$[rbp]
	movzx	eax, BYTE PTR [rcx+rax+8]
	movsxd	rcx, DWORD PTR i$1[rbp]
	mov	rdx, QWORD PTR g2$[rbp]
	movzx	ecx, BYTE PTR [rdx+rcx+8]
	cmp	eax, ecx
	je	SHORT $LN6@AuGUIDVeri

; 50   : 			return false;

	xor	al, al
	jmp	SHORT $LN1@AuGUIDVeri
$LN6@AuGUIDVeri:

; 51   : 	}

	jmp	SHORT $LN2@AuGUIDVeri
$LN3@AuGUIDVeri:

; 52   : 
; 53   : 	return true;

	mov	al, 1
$LN1@AuGUIDVeri:

; 54   : }

	lea	rsp, QWORD PTR [rbp+80]
	pop	rbp
	ret	0
AuGUIDVerify ENDP
_TEXT	ENDS
END
