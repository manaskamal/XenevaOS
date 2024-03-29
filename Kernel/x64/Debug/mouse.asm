; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?__ps2mouse@@3PEAU__ps2mouse__@@EA		; __ps2mouse
_BSS	SEGMENT
?__ps2mouse@@3PEAU__ps2mouse__@@EA DQ 01H DUP (?)	; __ps2mouse
_BSS	ENDS
CONST	SEGMENT
$SG3906	DB	'Right clicked ', 0dH, 0aH, 00H
	ORG $+7
$SG3929	DB	'PS2 Mouse with scroll wheel ', 0aH, 00H
CONST	ENDS
PUBLIC	?AuPS2MouseInitialise@@YAXXZ			; AuPS2MouseInitialise
PUBLIC	?AuPS2MouseSetPos@@YAXHH@Z			; AuPS2MouseSetPos
PUBLIC	?PS2MouseWaitInput@@YAXXZ			; PS2MouseWaitInput
PUBLIC	?PS2MouseWaitOutput@@YAXXZ			; PS2MouseWaitOutput
PUBLIC	?AuPS2MouseWrite@@YAEE@Z			; AuPS2MouseWrite
PUBLIC	?AuPS2MouseRead@@YAEXZ				; AuPS2MouseRead
PUBLIC	?PS2MouseHandler@@YAX_KPEAX@Z			; PS2MouseHandler
EXTRN	AuHalRegisterIRQ:PROC
EXTRN	AuInterruptEnd:PROC
EXTRN	x64_cli:PROC
EXTRN	x64_sti:PROC
EXTRN	x64_inportb:PROC
EXTRN	x64_outportb:PROC
EXTRN	kmalloc:PROC
EXTRN	memset:PROC
EXTRN	memcpy:PROC
EXTRN	AuTextOut:PROC
EXTRN	AuDevWriteMice:PROC
EXTRN	SeTextOut:PROC
pdata	SEGMENT
$pdata$?AuPS2MouseInitialise@@YAXXZ DD imagerel $LN4
	DD	imagerel $LN4+330
	DD	imagerel $unwind$?AuPS2MouseInitialise@@YAXXZ
$pdata$?PS2MouseWaitInput@@YAXXZ DD imagerel $LN6
	DD	imagerel $LN6+62
	DD	imagerel $unwind$?PS2MouseWaitInput@@YAXXZ
$pdata$?PS2MouseWaitOutput@@YAXXZ DD imagerel $LN6
	DD	imagerel $LN6+57
	DD	imagerel $unwind$?PS2MouseWaitOutput@@YAXXZ
$pdata$?AuPS2MouseWrite@@YAEE@Z DD imagerel $LN3
	DD	imagerel $LN3+62
	DD	imagerel $unwind$?AuPS2MouseWrite@@YAEE@Z
$pdata$?AuPS2MouseRead@@YAEXZ DD imagerel $LN3
	DD	imagerel $LN3+23
	DD	imagerel $unwind$?AuPS2MouseRead@@YAEXZ
$pdata$?PS2MouseHandler@@YAX_KPEAX@Z DD imagerel $LN31
	DD	imagerel $LN31+1248
	DD	imagerel $unwind$?PS2MouseHandler@@YAX_KPEAX@Z
pdata	ENDS
xdata	SEGMENT
$unwind$?AuPS2MouseInitialise@@YAXXZ DD 010401H
	DD	06204H
$unwind$?PS2MouseWaitInput@@YAXXZ DD 010401H
	DD	06204H
$unwind$?PS2MouseWaitOutput@@YAXXZ DD 010401H
	DD	06204H
$unwind$?AuPS2MouseWrite@@YAEE@Z DD 010801H
	DD	04208H
$unwind$?AuPS2MouseRead@@YAEXZ DD 010401H
	DD	04204H
$unwind$?PS2MouseHandler@@YAX_KPEAX@Z DD 010e01H
	DD	0a20eH
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\drivers\mouse.cpp
_TEXT	SEGMENT
mouse_in$1 = 32
status$ = 33
tv76 = 36
x$2 = 40
y$3 = 44
newmsg$4 = 48
v$ = 96
p$ = 104
?PS2MouseHandler@@YAX_KPEAX@Z PROC			; PS2MouseHandler

; 109  : void PS2MouseHandler(size_t v, void* p) {

$LN31:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 88					; 00000058H

; 110  : 	x64_cli();

	call	x64_cli

; 111  : 	uint8_t status = x64_inportb(MOUSE_STATUS);

	mov	cx, 100					; 00000064H
	call	x64_inportb
	mov	BYTE PTR status$[rsp], al
$LN28@PS2MouseHa:

; 112  : 	while ((status & MOUSE_BBIT) && (status & MOUSE_F_BIT)) {

	movzx	eax, BYTE PTR status$[rsp]
	and	eax, 1
	test	eax, eax
	je	$LN27@PS2MouseHa
	movzx	eax, BYTE PTR status$[rsp]
	and	eax, 32					; 00000020H
	test	eax, eax
	je	$LN27@PS2MouseHa

; 113  : 		int8_t mouse_in = x64_inportb(MOUSE_PORT);

	mov	cx, 96					; 00000060H
	call	x64_inportb
	mov	BYTE PTR mouse_in$1[rsp], al

; 114  : 		switch (__ps2mouse->mouse_cycle) {

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rax]
	mov	BYTE PTR tv76[rsp], al
	cmp	BYTE PTR tv76[rsp], 0
	je	SHORT $LN24@PS2MouseHa
	cmp	BYTE PTR tv76[rsp], 1
	je	SHORT $LN22@PS2MouseHa
	cmp	BYTE PTR tv76[rsp], 2
	je	$LN21@PS2MouseHa
	cmp	BYTE PTR tv76[rsp], 3
	je	$LN18@PS2MouseHa
	cmp	BYTE PTR tv76[rsp], 4
	je	$LN16@PS2MouseHa
	jmp	$LN25@PS2MouseHa
$LN24@PS2MouseHa:

; 115  : 		case 0:
; 116  : 			__ps2mouse->mouse_byte[0] = mouse_in;

	mov	eax, 1
	imul	rax, rax, 0
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	edx, BYTE PTR mouse_in$1[rsp]
	mov	BYTE PTR [rcx+rax+1], dl

; 117  : 			if (!(mouse_in & MOUSE_V_BIT)) break;

	movsx	eax, BYTE PTR mouse_in$1[rsp]
	and	eax, 8
	test	eax, eax
	jne	SHORT $LN23@PS2MouseHa
	jmp	$LN25@PS2MouseHa
$LN23@PS2MouseHa:

; 118  : 			++__ps2mouse->mouse_cycle;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rax]
	inc	al
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	BYTE PTR [rcx], al

; 119  : 			break;

	jmp	$LN25@PS2MouseHa
$LN22@PS2MouseHa:

; 120  : 		case 1:
; 121  : 			__ps2mouse->mouse_byte[1] = mouse_in;

	mov	eax, 1
	imul	rax, rax, 1
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	edx, BYTE PTR mouse_in$1[rsp]
	mov	BYTE PTR [rcx+rax+1], dl

; 122  : 			++__ps2mouse->mouse_cycle;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rax]
	inc	al
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	BYTE PTR [rcx], al

; 123  : 			break;

	jmp	SHORT $LN25@PS2MouseHa
$LN21@PS2MouseHa:

; 124  : 		case 2:
; 125  : 			__ps2mouse->mouse_byte[2] = mouse_in;

	mov	eax, 1
	imul	rax, rax, 2
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	edx, BYTE PTR mouse_in$1[rsp]
	mov	BYTE PTR [rcx+rax+1], dl

; 126  : 			if (__ps2mouse->mouse_mode == MOUSE_SCROLLWHEEL){

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rax+31]
	cmp	eax, 1
	jne	SHORT $LN20@PS2MouseHa

; 127  : 				++__ps2mouse->mouse_cycle;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rax]
	inc	al
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	BYTE PTR [rcx], al

; 128  : 				break;

	jmp	SHORT $LN25@PS2MouseHa
$LN20@PS2MouseHa:

; 129  : 			}
; 130  : 			goto finish_packet;

	jmp	SHORT $LN19@PS2MouseHa
	jmp	SHORT $finish_packet$32
$LN18@PS2MouseHa:

; 131  : 		case 3:
; 132  : 			__ps2mouse->mouse_byte[3] = mouse_in;

	mov	eax, 1
	imul	rax, rax, 3
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	edx, BYTE PTR mouse_in$1[rsp]
	mov	BYTE PTR [rcx+rax+1], dl

; 133  : 			goto finish_packet;

	jmp	SHORT $LN17@PS2MouseHa
	jmp	SHORT $finish_packet$32
$LN16@PS2MouseHa:

; 134  : 		case 4:
; 135  : 			__ps2mouse->mouse_byte[4] = mouse_in;

	mov	eax, 1
	imul	rax, rax, 4
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	edx, BYTE PTR mouse_in$1[rsp]
	mov	BYTE PTR [rcx+rax+1], dl

; 136  : 			goto finish_packet;

	jmp	SHORT $LN15@PS2MouseHa
	jmp	SHORT $finish_packet$32
$LN25@PS2MouseHa:

; 137  : 		}
; 138  : 
; 139  : 		goto read_next;

	jmp	$LN14@PS2MouseHa
	jmp	$read_next$33
$LN15@PS2MouseHa:
$LN17@PS2MouseHa:
$LN19@PS2MouseHa:
$finish_packet$32:

; 140  : 
; 141  : 	finish_packet:
; 142  : 		__ps2mouse->mouse_cycle = 0;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	BYTE PTR [rax], 0

; 143  : 
; 144  : 		int x = __ps2mouse->mouse_byte[1];

	mov	eax, 1
	imul	rax, rax, 1
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rcx+rax+1]
	mov	DWORD PTR x$2[rsp], eax

; 145  : 		int y = __ps2mouse->mouse_byte[2];

	mov	eax, 1
	imul	rax, rax, 2
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rcx+rax+1]
	mov	DWORD PTR y$3[rsp], eax

; 146  : 		if (x && __ps2mouse->mouse_byte[0] & (1 << 4))

	cmp	DWORD PTR x$2[rsp], 0
	je	SHORT $LN13@PS2MouseHa
	mov	eax, 1
	imul	rax, rax, 0
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rcx+rax+1]
	and	eax, 16
	test	eax, eax
	je	SHORT $LN13@PS2MouseHa

; 147  : 			x = x - 0x100;

	mov	eax, DWORD PTR x$2[rsp]
	sub	eax, 256				; 00000100H
	mov	DWORD PTR x$2[rsp], eax
$LN13@PS2MouseHa:

; 148  : 
; 149  : 		if (y && __ps2mouse->mouse_byte[0] & (1 << 5))

	cmp	DWORD PTR y$3[rsp], 0
	je	SHORT $LN12@PS2MouseHa
	mov	eax, 1
	imul	rax, rax, 0
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rcx+rax+1]
	and	eax, 32					; 00000020H
	test	eax, eax
	je	SHORT $LN12@PS2MouseHa

; 150  : 			y = y - 0x100;

	mov	eax, DWORD PTR y$3[rsp]
	sub	eax, 256				; 00000100H
	mov	DWORD PTR y$3[rsp], eax
$LN12@PS2MouseHa:

; 151  : 
; 152  : 		__ps2mouse->mouse_x_diff = x;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	ecx, DWORD PTR x$2[rsp]
	mov	DWORD PTR [rax+13], ecx

; 153  : 		__ps2mouse->mouse_y_diff = y;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	ecx, DWORD PTR y$3[rsp]
	mov	DWORD PTR [rax+17], ecx

; 154  : 		__ps2mouse->mouse_x += __ps2mouse->mouse_x_diff;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	eax, DWORD PTR [rax+5]
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	add	eax, DWORD PTR [rcx+13]
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rcx+5], eax

; 155  : 		__ps2mouse->mouse_y -= __ps2mouse->mouse_y_diff;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	ecx, DWORD PTR [rcx+17]
	mov	eax, DWORD PTR [rax+9]
	sub	eax, ecx
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rcx+9], eax

; 156  : 
; 157  : 		if (__ps2mouse->mouse_x < 0)

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	cmp	DWORD PTR [rax+5], 0
	jge	SHORT $LN11@PS2MouseHa

; 158  : 			__ps2mouse->mouse_x = 0;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rax+5], 0
$LN11@PS2MouseHa:

; 159  : 
; 160  : 		if (__ps2mouse->mouse_y < 0)

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	cmp	DWORD PTR [rax+9], 0
	jge	SHORT $LN10@PS2MouseHa

; 161  : 			__ps2mouse->mouse_y = 0;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rax+9], 0
$LN10@PS2MouseHa:

; 162  : 
; 163  : 		__ps2mouse->mouse_butt_state = 0;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rax+27], 0

; 164  : 
; 165  : 		if (__ps2mouse->mouse_byte[0] & 0x01) {    //0x01 for PS/2

	mov	eax, 1
	imul	rax, rax, 0
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rcx+rax+1]
	and	eax, 1
	test	eax, eax
	je	SHORT $LN9@PS2MouseHa

; 166  : 			__ps2mouse->curr_button[0] = 1;

	mov	eax, 1
	imul	rax, rax, 0
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	BYTE PTR [rcx+rax+24], 1

; 167  : 			__ps2mouse->mouse_butt_state |= LEFT_CLICK;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	eax, DWORD PTR [rax+27]
	or	eax, 1
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rcx+27], eax

; 168  : 		}
; 169  : 		else

	jmp	SHORT $LN8@PS2MouseHa
$LN9@PS2MouseHa:

; 170  : 			__ps2mouse->curr_button[0] = 0;

	mov	eax, 1
	imul	rax, rax, 0
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	BYTE PTR [rcx+rax+24], 0
$LN8@PS2MouseHa:

; 171  : 
; 172  : 		if (__ps2mouse->mouse_byte[0] & 0x02) {

	mov	eax, 1
	imul	rax, rax, 0
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rcx+rax+1]
	and	eax, 2
	test	eax, eax
	je	SHORT $LN7@PS2MouseHa

; 173  : 			__ps2mouse->curr_button[2] = 1;

	mov	eax, 1
	imul	rax, rax, 2
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	BYTE PTR [rcx+rax+24], 1

; 174  : 			SeTextOut("Right clicked \r\n");

	lea	rcx, OFFSET FLAT:$SG3906
	call	SeTextOut

; 175  : 			__ps2mouse->mouse_butt_state |= RIGHT_CLICK;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	eax, DWORD PTR [rax+27]
	or	eax, 2
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rcx+27], eax

; 176  : 		}
; 177  : 		else

	jmp	SHORT $LN6@PS2MouseHa
$LN7@PS2MouseHa:

; 178  : 			__ps2mouse->curr_button[2] = 0;

	mov	eax, 1
	imul	rax, rax, 2
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	BYTE PTR [rcx+rax+24], 0
$LN6@PS2MouseHa:

; 179  : 
; 180  : 		if (__ps2mouse->mouse_byte[0] & 0x04)

	mov	eax, 1
	imul	rax, rax, 0
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rcx+rax+1]
	and	eax, 4
	test	eax, eax
	je	SHORT $LN5@PS2MouseHa

; 181  : 			__ps2mouse->mouse_butt_state |= MOUSE_MIDDLE_CLICK;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	eax, DWORD PTR [rax+27]
	or	eax, 4
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rcx+27], eax
$LN5@PS2MouseHa:

; 182  : 
; 183  : 		if (__ps2mouse->mouse_byte[3]){

	mov	eax, 1
	imul	rax, rax, 3
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rcx+rax+1]
	test	eax, eax
	je	SHORT $LN4@PS2MouseHa

; 184  : 			if ((int8_t)__ps2mouse->mouse_byte[3] > 0)

	mov	eax, 1
	imul	rax, rax, 3
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movsx	eax, BYTE PTR [rcx+rax+1]
	test	eax, eax
	jle	SHORT $LN3@PS2MouseHa

; 185  : 				__ps2mouse->mouse_butt_state |= MOUSE_SCROLL_DOWN;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	eax, DWORD PTR [rax+27]
	or	eax, 6
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rcx+27], eax
	jmp	SHORT $LN2@PS2MouseHa
$LN3@PS2MouseHa:

; 186  : 			else if ((int8_t)__ps2mouse->mouse_byte[3] < 0)

	mov	eax, 1
	imul	rax, rax, 3
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movsx	eax, BYTE PTR [rcx+rax+1]
	test	eax, eax
	jge	SHORT $LN1@PS2MouseHa

; 187  : 				__ps2mouse->mouse_butt_state |= MOUSE_SCROLL_UP;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	eax, DWORD PTR [rax+27]
	or	eax, 5
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rcx+27], eax
$LN1@PS2MouseHa:
$LN2@PS2MouseHa:
$LN4@PS2MouseHa:

; 188  : 		}
; 189  : 
; 190  : 
; 191  : 		AuInputMessage newmsg;
; 192  : 		memset(&newmsg, 0, sizeof(AuInputMessage));

	mov	r8d, 26
	xor	edx, edx
	lea	rcx, QWORD PTR newmsg$4[rsp]
	call	memset

; 193  : 		newmsg.type = AU_INPUT_MOUSE;

	mov	BYTE PTR newmsg$4[rsp], 1

; 194  : 		newmsg.xpos = __ps2mouse->mouse_x;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	eax, DWORD PTR [rax+5]
	mov	DWORD PTR newmsg$4[rsp+1], eax

; 195  : 		newmsg.ypos = __ps2mouse->mouse_y;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	eax, DWORD PTR [rax+9]
	mov	DWORD PTR newmsg$4[rsp+5], eax

; 196  : 		newmsg.button_state = __ps2mouse->mouse_butt_state;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	movzx	eax, BYTE PTR [rax+27]
	mov	BYTE PTR newmsg$4[rsp+9], al

; 197  : 
; 198  : 		/*AuInputMessage oldmsg;
; 199  : 		AuDevReadMice(&oldmsg);*/
; 200  : 
; 201  : 		AuDevWriteMice(&newmsg);

	lea	rcx, QWORD PTR newmsg$4[rsp]
	call	AuDevWriteMice

; 202  : 		memcpy(__ps2mouse->prev_button, __ps2mouse->curr_button, 3);

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	add	rax, 24
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	add	rcx, 21
	mov	r8d, 3
	mov	rdx, rax
	call	memcpy

; 203  : 		memset(__ps2mouse->curr_button, 0x00, 3);

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	add	rax, 24
	mov	r8d, 3
	xor	edx, edx
	mov	rcx, rax
	call	memset
$LN14@PS2MouseHa:
$read_next$33:

; 204  : 
; 205  : 	read_next:
; 206  : 		break;

	jmp	SHORT $LN27@PS2MouseHa

; 207  : 	}

	jmp	$LN28@PS2MouseHa
$LN27@PS2MouseHa:

; 208  : 
; 209  : 	x64_sti();

	call	x64_sti

; 210  : 	AuInterruptEnd(12);

	mov	cl, 12
	call	AuInterruptEnd

; 211  : }

	add	rsp, 88					; 00000058H
	ret	0
?PS2MouseHandler@@YAX_KPEAX@Z ENDP			; PS2MouseHandler
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\drivers\mouse.cpp
_TEXT	SEGMENT
?AuPS2MouseRead@@YAEXZ PROC				; AuPS2MouseRead

; 104  : uint8_t AuPS2MouseRead() {

$LN3:
	sub	rsp, 40					; 00000028H

; 105  : 	PS2MouseWaitOutput();

	call	?PS2MouseWaitOutput@@YAXXZ		; PS2MouseWaitOutput

; 106  : 	return x64_inportb(0x60);

	mov	cx, 96					; 00000060H
	call	x64_inportb

; 107  : }

	add	rsp, 40					; 00000028H
	ret	0
?AuPS2MouseRead@@YAEXZ ENDP				; AuPS2MouseRead
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\drivers\mouse.cpp
_TEXT	SEGMENT
write$ = 48
?AuPS2MouseWrite@@YAEE@Z PROC				; AuPS2MouseWrite

; 95   : uint8_t AuPS2MouseWrite(uint8_t write) {

$LN3:
	mov	BYTE PTR [rsp+8], cl
	sub	rsp, 40					; 00000028H

; 96   : 	PS2MouseWaitInput();

	call	?PS2MouseWaitInput@@YAXXZ		; PS2MouseWaitInput

; 97   : 	x64_outportb(0x64, 0xD4);

	mov	dl, 212					; 000000d4H
	mov	cx, 100					; 00000064H
	call	x64_outportb

; 98   : 	PS2MouseWaitInput();

	call	?PS2MouseWaitInput@@YAXXZ		; PS2MouseWaitInput

; 99   : 	x64_outportb(0x60, write);

	movzx	edx, BYTE PTR write$[rsp]
	mov	cx, 96					; 00000060H
	call	x64_outportb

; 100  : 	PS2MouseWaitOutput();

	call	?PS2MouseWaitOutput@@YAXXZ		; PS2MouseWaitOutput

; 101  : 	return x64_inportb(0x60);

	mov	cx, 96					; 00000060H
	call	x64_inportb

; 102  : }

	add	rsp, 40					; 00000028H
	ret	0
?AuPS2MouseWrite@@YAEE@Z ENDP				; AuPS2MouseWrite
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\drivers\mouse.cpp
_TEXT	SEGMENT
_timer_out$ = 32
?PS2MouseWaitOutput@@YAXXZ PROC				; PS2MouseWaitOutput

; 83   : void PS2MouseWaitOutput() {

$LN6:
	sub	rsp, 56					; 00000038H

; 84   : 	uint32_t _timer_out = 100000;

	mov	DWORD PTR _timer_out$[rsp], 100000	; 000186a0H
$LN3@PS2MouseWa:

; 85   : 	while (--_timer_out) {

	mov	eax, DWORD PTR _timer_out$[rsp]
	dec	eax
	mov	DWORD PTR _timer_out$[rsp], eax
	cmp	DWORD PTR _timer_out$[rsp], 0
	je	SHORT $LN2@PS2MouseWa

; 86   : 		if (x64_inportb(0x64) & (1<<0))

	mov	cx, 100					; 00000064H
	call	x64_inportb
	movzx	eax, al
	and	eax, 1
	test	eax, eax
	je	SHORT $LN1@PS2MouseWa

; 87   : 			return;

	jmp	SHORT $LN4@PS2MouseWa
$LN1@PS2MouseWa:

; 88   : 	}

	jmp	SHORT $LN3@PS2MouseWa
$LN2@PS2MouseWa:
$LN4@PS2MouseWa:

; 89   : }

	add	rsp, 56					; 00000038H
	ret	0
?PS2MouseWaitOutput@@YAXXZ ENDP				; PS2MouseWaitOutput
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\drivers\mouse.cpp
_TEXT	SEGMENT
time_out$ = 32
?PS2MouseWaitInput@@YAXXZ PROC				; PS2MouseWaitInput

; 72   : void PS2MouseWaitInput() {

$LN6:
	sub	rsp, 56					; 00000038H

; 73   : 	uint64_t time_out = 100000UL;

	mov	QWORD PTR time_out$[rsp], 100000	; 000186a0H
$LN3@PS2MouseWa:

; 74   : 	while (--time_out){

	mov	rax, QWORD PTR time_out$[rsp]
	dec	rax
	mov	QWORD PTR time_out$[rsp], rax
	cmp	QWORD PTR time_out$[rsp], 0
	je	SHORT $LN2@PS2MouseWa

; 75   : 		if (!(x64_inportb(0x64) & (1 << 1))) return;

	mov	cx, 100					; 00000064H
	call	x64_inportb
	movzx	eax, al
	and	eax, 2
	test	eax, eax
	jne	SHORT $LN1@PS2MouseWa
	jmp	SHORT $LN4@PS2MouseWa
$LN1@PS2MouseWa:

; 76   : 	}

	jmp	SHORT $LN3@PS2MouseWa
$LN2@PS2MouseWa:
$LN4@PS2MouseWa:

; 77   : }

	add	rsp, 56					; 00000038H
	ret	0
?PS2MouseWaitInput@@YAXXZ ENDP				; PS2MouseWaitInput
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\drivers\mouse.cpp
_TEXT	SEGMENT
x$ = 8
y$ = 16
?AuPS2MouseSetPos@@YAXHH@Z PROC				; AuPS2MouseSetPos

; 220  : void AuPS2MouseSetPos(int32_t x, int32_t y) {

	mov	DWORD PTR [rsp+16], edx
	mov	DWORD PTR [rsp+8], ecx

; 221  : 	if (!__ps2mouse)

	cmp	QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA, 0 ; __ps2mouse
	jne	SHORT $LN1@AuPS2Mouse

; 222  : 		return;

	jmp	SHORT $LN2@AuPS2Mouse
$LN1@AuPS2Mouse:

; 223  : 	__ps2mouse->mouse_x = x;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	ecx, DWORD PTR x$[rsp]
	mov	DWORD PTR [rax+5], ecx

; 224  : 	__ps2mouse->mouse_y = y;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	ecx, DWORD PTR y$[rsp]
	mov	DWORD PTR [rax+9], ecx
$LN2@AuPS2Mouse:

; 225  : }

	ret	0
?AuPS2MouseSetPos@@YAXHH@Z ENDP				; AuPS2MouseSetPos
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\drivers\mouse.cpp
_TEXT	SEGMENT
status$ = 32
?AuPS2MouseInitialise@@YAXXZ PROC			; AuPS2MouseInitialise

; 230  : void AuPS2MouseInitialise() {

$LN4:
	sub	rsp, 56					; 00000038H

; 231  : 	__ps2mouse = (PS2Mouse*)kmalloc(sizeof(PS2Mouse));

	mov	ecx, 36					; 00000024H
	call	kmalloc
	mov	QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA, rax ; __ps2mouse

; 232  : 	memset(__ps2mouse, 0, sizeof(PS2Mouse));

	mov	r8d, 36					; 00000024H
	xor	edx, edx
	mov	rcx, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	call	memset

; 233  : 
; 234  : 	__ps2mouse->mouse_mode = MOUSE_DEFAULT;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	BYTE PTR [rax+31], 0

; 235  : 	uint8_t status;
; 236  : 
; 237  : 	PS2MouseWaitInput();

	call	?PS2MouseWaitInput@@YAXXZ		; PS2MouseWaitInput

; 238  : 	x64_outportb(0x64, 0xA8);

	mov	dl, 168					; 000000a8H
	mov	cx, 100					; 00000064H
	call	x64_outportb

; 239  : 
; 240  : 	PS2MouseWaitInput();

	call	?PS2MouseWaitInput@@YAXXZ		; PS2MouseWaitInput

; 241  : 	x64_outportb(0x64, 0x20);

	mov	dl, 32					; 00000020H
	mov	cx, 100					; 00000064H
	call	x64_outportb

; 242  : 
; 243  : 	PS2MouseWaitInput();

	call	?PS2MouseWaitInput@@YAXXZ		; PS2MouseWaitInput

; 244  : 	status = (x64_inportb(0x60) | 2);

	mov	cx, 96					; 00000060H
	call	x64_inportb
	movzx	eax, al
	or	eax, 2
	mov	BYTE PTR status$[rsp], al

; 245  : 
; 246  : 	PS2MouseWaitInput();

	call	?PS2MouseWaitInput@@YAXXZ		; PS2MouseWaitInput

; 247  : 	x64_outportb(0x64, 0x60);

	mov	dl, 96					; 00000060H
	mov	cx, 100					; 00000064H
	call	x64_outportb

; 248  : 
; 249  : 	PS2MouseWaitInput();

	call	?PS2MouseWaitInput@@YAXXZ		; PS2MouseWaitInput

; 250  : 	x64_outportb(0x60, status);

	movzx	edx, BYTE PTR status$[rsp]
	mov	cx, 96					; 00000060H
	call	x64_outportb

; 251  : 
; 252  : 	AuPS2MouseWrite(0xF6);

	mov	cl, 246					; 000000f6H
	call	?AuPS2MouseWrite@@YAEE@Z		; AuPS2MouseWrite

; 253  : 	AuPS2MouseRead();

	call	?AuPS2MouseRead@@YAEXZ			; AuPS2MouseRead

; 254  : 
; 255  : 	AuPS2MouseWrite(0xF4);

	mov	cl, 244					; 000000f4H
	call	?AuPS2MouseWrite@@YAEE@Z		; AuPS2MouseWrite

; 256  : 	AuPS2MouseRead();

	call	?AuPS2MouseRead@@YAEXZ			; AuPS2MouseRead

; 257  : 
; 258  : 	/* Enable the scroll wheel */
; 259  : 	AuPS2MouseWrite(MOUSE_DEVICE_ID);

	mov	cl, 242					; 000000f2H
	call	?AuPS2MouseWrite@@YAEE@Z		; AuPS2MouseWrite

; 260  : 	status = AuPS2MouseRead();

	call	?AuPS2MouseRead@@YAEXZ			; AuPS2MouseRead
	mov	BYTE PTR status$[rsp], al

; 261  : 	AuPS2MouseWrite(MOUSE_SAMPLE_RATE);

	mov	cl, 243					; 000000f3H
	call	?AuPS2MouseWrite@@YAEE@Z		; AuPS2MouseWrite

; 262  : 	AuPS2MouseWrite(200);

	mov	cl, 200					; 000000c8H
	call	?AuPS2MouseWrite@@YAEE@Z		; AuPS2MouseWrite

; 263  : 	AuPS2MouseWrite(MOUSE_SAMPLE_RATE);

	mov	cl, 243					; 000000f3H
	call	?AuPS2MouseWrite@@YAEE@Z		; AuPS2MouseWrite

; 264  : 	AuPS2MouseWrite(100);

	mov	cl, 100					; 00000064H
	call	?AuPS2MouseWrite@@YAEE@Z		; AuPS2MouseWrite

; 265  : 	AuPS2MouseWrite(MOUSE_SAMPLE_RATE);

	mov	cl, 243					; 000000f3H
	call	?AuPS2MouseWrite@@YAEE@Z		; AuPS2MouseWrite

; 266  : 	AuPS2MouseWrite(80);

	mov	cl, 80					; 00000050H
	call	?AuPS2MouseWrite@@YAEE@Z		; AuPS2MouseWrite

; 267  : 	AuPS2MouseWrite(MOUSE_DEVICE_ID);

	mov	cl, 242					; 000000f2H
	call	?AuPS2MouseWrite@@YAEE@Z		; AuPS2MouseWrite

; 268  : 	status = AuPS2MouseRead();

	call	?AuPS2MouseRead@@YAEXZ			; AuPS2MouseRead
	mov	BYTE PTR status$[rsp], al

; 269  : 	if (status == 3) {

	movzx	eax, BYTE PTR status$[rsp]
	cmp	eax, 3
	jne	SHORT $LN1@AuPS2Mouse

; 270  : 		AuTextOut("PS2 Mouse with scroll wheel \n");

	lea	rcx, OFFSET FLAT:$SG3929
	call	AuTextOut

; 271  : 		__ps2mouse->mouse_mode = MOUSE_SCROLLWHEEL;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	BYTE PTR [rax+31], 1
$LN1@AuPS2Mouse:

; 272  : 	}
; 273  : 
; 274  : 	__ps2mouse->mouse_x = 0;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rax+5], 0

; 275  : 	__ps2mouse->mouse_y = 0;

	mov	rax, QWORD PTR ?__ps2mouse@@3PEAU__ps2mouse__@@EA ; __ps2mouse
	mov	DWORD PTR [rax+9], 0

; 276  : 
; 277  : 	AuHalRegisterIRQ(34, PS2MouseHandler, 12, false);  //34

	xor	r9d, r9d
	mov	r8b, 12
	lea	rdx, OFFSET FLAT:?PS2MouseHandler@@YAX_KPEAX@Z ; PS2MouseHandler
	mov	ecx, 34					; 00000022H
	call	AuHalRegisterIRQ

; 278  : }

	add	rsp, 56					; 00000038H
	ret	0
?AuPS2MouseInitialise@@YAXXZ ENDP			; AuPS2MouseInitialise
_TEXT	ENDS
END
