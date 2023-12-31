; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	?integer_buffer@@3PADA				; integer_buffer
PUBLIC	?float_to_string_output@@3PADA			; float_to_string_output
_BSS	SEGMENT
?integer_buffer@@3PADA DB 020H DUP (?)			; integer_buffer
?float_to_string_output@@3PADA DB 020H DUP (?)		; float_to_string_output
_BSS	ENDS
CONST	SEGMENT
$SG2752	DB	'0123456789ABCDEF', 00H
CONST	ENDS
_DATA	SEGMENT
chars	DQ	FLAT:$SG2752
r_x	DD	075bcd15H
r_y	DD	0159a55e5H
r_z	DD	01f123bb5H
r_w	DD	05491333H
_DATA	ENDS
PUBLIC	?atoi@@YAHPEBD@Z				; atoi
PUBLIC	?sztoa@@YAPEAD_KPEADH@Z				; sztoa
PUBLIC	printf
PUBLIC	?ftoa@@YAPEADME@Z				; ftoa
PUBLIC	?rand@@YAHXZ					; rand
PUBLIC	?srand@@YAXI@Z					; srand
PUBLIC	?atow@@YAXPEADPEBD@Z				; atow
PUBLIC	?int_to_str@@YAPEBDH@Z				; int_to_str
PUBLIC	__real@41200000
PUBLIC	__real@bf800000
EXTRN	AuTextOut:PROC
EXTRN	isspace:PROC
EXTRN	isdigit:PROC
EXTRN	_fltused:DWORD
pdata	SEGMENT
$pdata$?atoi@@YAHPEBD@Z DD imagerel $LN13
	DD	imagerel $LN13+200
	DD	imagerel $unwind$?atoi@@YAHPEBD@Z
$pdata$?sztoa@@YAPEAD_KPEADH@Z DD imagerel $LN11
	DD	imagerel $LN11+275
	DD	imagerel $unwind$?sztoa@@YAPEAD_KPEADH@Z
$pdata$printf DD imagerel $LN3
	DD	imagerel $LN3+39
	DD	imagerel $unwind$printf
$pdata$?ftoa@@YAPEADME@Z DD imagerel $LN9
	DD	imagerel $LN9+311
	DD	imagerel $unwind$?ftoa@@YAPEADME@Z
$pdata$?rand@@YAHXZ DD imagerel $LN3
	DD	imagerel $LN3+113
	DD	imagerel $unwind$?rand@@YAHXZ
$pdata$?int_to_str@@YAPEBDH@Z DD imagerel $LN7
	DD	imagerel $LN7+280
	DD	imagerel $unwind$?int_to_str@@YAPEBDH@Z
pdata	ENDS
;	COMDAT __real@bf800000
CONST	SEGMENT
__real@bf800000 DD 0bf800000r			; -1
CONST	ENDS
;	COMDAT __real@41200000
CONST	SEGMENT
__real@41200000 DD 041200000r			; 10
CONST	ENDS
xdata	SEGMENT
$unwind$?atoi@@YAHPEBD@Z DD 010901H
	DD	06209H
$unwind$?sztoa@@YAPEAD_KPEADH@Z DD 011301H
	DD	04213H
$unwind$printf DD 011801H
	DD	04218H
$unwind$?ftoa@@YAPEADME@Z DD 010e01H
	DD	0820eH
$unwind$?rand@@YAHXZ DD 010401H
	DD	02204H
$unwind$?int_to_str@@YAPEBDH@Z DD 010801H
	DD	04208H
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\stdio.cpp
_TEXT	SEGMENT
size$ = 0
index$ = 1
remainder$1 = 2
remainder$ = 3
new_value$ = 8
size_tester$ = 16
value$ = 48
?int_to_str@@YAPEBDH@Z PROC				; int_to_str

; 50   : const char* int_to_str(int value) {

$LN7:
	mov	DWORD PTR [rsp+8], ecx
	sub	rsp, 40					; 00000028H

; 51   : 	uint8_t size = 0;

	mov	BYTE PTR size$[rsp], 0

; 52   : 	uint64_t size_tester = (uint64_t)value;

	movsxd	rax, DWORD PTR value$[rsp]
	mov	QWORD PTR size_tester$[rsp], rax
$LN4@int_to_str:

; 53   : 	while (size_tester / 10 > 0) {

	xor	edx, edx
	mov	rax, QWORD PTR size_tester$[rsp]
	mov	ecx, 10
	div	rcx
	test	rax, rax
	jbe	SHORT $LN3@int_to_str

; 54   : 		size_tester /= 10;

	xor	edx, edx
	mov	rax, QWORD PTR size_tester$[rsp]
	mov	ecx, 10
	div	rcx
	mov	QWORD PTR size_tester$[rsp], rax

; 55   : 		size++;

	movzx	eax, BYTE PTR size$[rsp]
	inc	al
	mov	BYTE PTR size$[rsp], al

; 56   : 	}

	jmp	SHORT $LN4@int_to_str
$LN3@int_to_str:

; 57   : 
; 58   : 	uint8_t index = 0;

	mov	BYTE PTR index$[rsp], 0

; 59   : 	uint64_t new_value = (uint64_t)value;

	movsxd	rax, DWORD PTR value$[rsp]
	mov	QWORD PTR new_value$[rsp], rax
$LN2@int_to_str:

; 60   : 	while (new_value / 10 > 0) {

	xor	edx, edx
	mov	rax, QWORD PTR new_value$[rsp]
	mov	ecx, 10
	div	rcx
	test	rax, rax
	jbe	SHORT $LN1@int_to_str

; 61   : 		uint8_t remainder = new_value % 10;

	xor	edx, edx
	mov	rax, QWORD PTR new_value$[rsp]
	mov	ecx, 10
	div	rcx
	mov	rax, rdx
	mov	BYTE PTR remainder$1[rsp], al

; 62   : 		new_value /= 10;

	xor	edx, edx
	mov	rax, QWORD PTR new_value$[rsp]
	mov	ecx, 10
	div	rcx
	mov	QWORD PTR new_value$[rsp], rax

; 63   : 		integer_buffer[size - index] = remainder + 48;

	movzx	eax, BYTE PTR remainder$1[rsp]
	add	eax, 48					; 00000030H
	movzx	ecx, BYTE PTR size$[rsp]
	movzx	edx, BYTE PTR index$[rsp]
	sub	ecx, edx
	movsxd	rcx, ecx
	lea	rdx, OFFSET FLAT:?integer_buffer@@3PADA	; integer_buffer
	mov	BYTE PTR [rdx+rcx], al

; 64   : 		index++;

	movzx	eax, BYTE PTR index$[rsp]
	inc	al
	mov	BYTE PTR index$[rsp], al

; 65   : 	}

	jmp	SHORT $LN2@int_to_str
$LN1@int_to_str:

; 66   : 
; 67   : 	uint8_t remainder = new_value % 10;

	xor	edx, edx
	mov	rax, QWORD PTR new_value$[rsp]
	mov	ecx, 10
	div	rcx
	mov	rax, rdx
	mov	BYTE PTR remainder$[rsp], al

; 68   : 	integer_buffer[size - index] = remainder + 48;

	movzx	eax, BYTE PTR remainder$[rsp]
	add	eax, 48					; 00000030H
	movzx	ecx, BYTE PTR size$[rsp]
	movzx	edx, BYTE PTR index$[rsp]
	sub	ecx, edx
	movsxd	rcx, ecx
	lea	rdx, OFFSET FLAT:?integer_buffer@@3PADA	; integer_buffer
	mov	BYTE PTR [rdx+rcx], al

; 69   : 	integer_buffer[size + 1] = 0;

	movzx	eax, BYTE PTR size$[rsp]
	inc	eax
	cdqe
	lea	rcx, OFFSET FLAT:?integer_buffer@@3PADA	; integer_buffer
	mov	BYTE PTR [rcx+rax], 0

; 70   : 	return integer_buffer;

	lea	rax, OFFSET FLAT:?integer_buffer@@3PADA	; integer_buffer

; 71   : }

	add	rsp, 40					; 00000028H
	ret	0
?int_to_str@@YAPEBDH@Z ENDP				; int_to_str
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\stdio.cpp
_TEXT	SEGMENT
buf$ = 8
source$ = 16
?atow@@YAXPEADPEBD@Z PROC				; atow

; 44   : {

	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
$LN2@atow:

; 45   : 	while (*source != 0)*buf++ = *source++;

	mov	rax, QWORD PTR source$[rsp]
	movsx	eax, BYTE PTR [rax]
	test	eax, eax
	je	SHORT $LN1@atow
	mov	rax, QWORD PTR buf$[rsp]
	mov	rcx, QWORD PTR source$[rsp]
	movzx	ecx, BYTE PTR [rcx]
	mov	BYTE PTR [rax], cl
	mov	rax, QWORD PTR buf$[rsp]
	inc	rax
	mov	QWORD PTR buf$[rsp], rax
	mov	rax, QWORD PTR source$[rsp]
	inc	rax
	mov	QWORD PTR source$[rsp], rax
	jmp	SHORT $LN2@atow
$LN1@atow:

; 46   : 	*buf = '\0';

	mov	rax, QWORD PTR buf$[rsp]
	mov	BYTE PTR [rax], 0

; 47   : }

	ret	0
?atow@@YAXPEADPEBD@Z ENDP				; atow
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\stdio.cpp
_TEXT	SEGMENT
seed$ = 8
?srand@@YAXI@Z PROC					; srand

; 142  : void srand(unsigned int seed) {

	mov	DWORD PTR [rsp+8], ecx

; 143  : 	r_w ^= seed;

	mov	eax, DWORD PTR seed$[rsp]
	mov	ecx, DWORD PTR r_w
	xor	ecx, eax
	mov	eax, ecx
	mov	DWORD PTR r_w, eax

; 144  : }

	ret	0
?srand@@YAXI@Z ENDP					; srand
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\stdio.cpp
_TEXT	SEGMENT
t$ = 0
?rand@@YAHXZ PROC					; rand

; 132  : int rand() {

$LN3:
	sub	rsp, 24

; 133  : 	uint32_t t;
; 134  : 
; 135  : 	t = r_x ^ (r_x << 11);

	mov	eax, DWORD PTR r_x
	shl	eax, 11
	mov	ecx, DWORD PTR r_x
	xor	ecx, eax
	mov	eax, ecx
	mov	DWORD PTR t$[rsp], eax

; 136  : 	r_x = r_y; r_y = r_z; r_z = r_w;

	mov	eax, DWORD PTR r_y
	mov	DWORD PTR r_x, eax
	mov	eax, DWORD PTR r_z
	mov	DWORD PTR r_y, eax
	mov	eax, DWORD PTR r_w
	mov	DWORD PTR r_z, eax

; 137  : 	r_w = r_w ^ (r_w >> 19) ^ t ^ (t >> 8);

	mov	eax, DWORD PTR r_w
	shr	eax, 19
	mov	ecx, DWORD PTR r_w
	xor	ecx, eax
	mov	eax, ecx
	xor	eax, DWORD PTR t$[rsp]
	mov	ecx, DWORD PTR t$[rsp]
	shr	ecx, 8
	xor	eax, ecx
	mov	DWORD PTR r_w, eax

; 138  : 
; 139  : 	return (r_w & RAND_MAX);

	mov	eax, DWORD PTR r_w
	btr	eax, 31

; 140  : }

	add	rsp, 24
	ret	0
?rand@@YAHXZ ENDP					; rand
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\stdio.cpp
_TEXT	SEGMENT
i$1 = 32
new_value$ = 36
float_ptr$ = 40
int_ptr$ = 48
value$ = 80
decimal_places$ = 88
?ftoa@@YAPEADME@Z PROC					; ftoa

; 74   : char* ftoa(float value, uint8_t decimal_places) {

$LN9:
	mov	BYTE PTR [rsp+16], dl
	movss	DWORD PTR [rsp+8], xmm0
	sub	rsp, 72					; 00000048H

; 75   : 	char* int_ptr = (char*)int_to_str((int)value);

	cvttss2si eax, DWORD PTR value$[rsp]
	mov	ecx, eax
	call	?int_to_str@@YAPEBDH@Z			; int_to_str
	mov	QWORD PTR int_ptr$[rsp], rax

; 76   : 	char* float_ptr = float_to_string_output;

	lea	rax, OFFSET FLAT:?float_to_string_output@@3PADA ; float_to_string_output
	mov	QWORD PTR float_ptr$[rsp], rax

; 77   : 
; 78   : 	if (value < 0) {

	xorps	xmm0, xmm0
	comiss	xmm0, DWORD PTR value$[rsp]
	jbe	SHORT $LN6@ftoa

; 79   : 		value *= -1;

	movss	xmm0, DWORD PTR value$[rsp]
	mulss	xmm0, DWORD PTR __real@bf800000
	movss	DWORD PTR value$[rsp], xmm0
$LN6@ftoa:
$LN5@ftoa:

; 80   : 	}
; 81   : 
; 82   : 
; 83   : 	while (*int_ptr != 0) {

	mov	rax, QWORD PTR int_ptr$[rsp]
	movsx	eax, BYTE PTR [rax]
	test	eax, eax
	je	SHORT $LN4@ftoa

; 84   : 		*float_ptr = *int_ptr;

	mov	rax, QWORD PTR float_ptr$[rsp]
	mov	rcx, QWORD PTR int_ptr$[rsp]
	movzx	ecx, BYTE PTR [rcx]
	mov	BYTE PTR [rax], cl

; 85   : 		int_ptr++;

	mov	rax, QWORD PTR int_ptr$[rsp]
	inc	rax
	mov	QWORD PTR int_ptr$[rsp], rax

; 86   : 		float_ptr++;

	mov	rax, QWORD PTR float_ptr$[rsp]
	inc	rax
	mov	QWORD PTR float_ptr$[rsp], rax

; 87   : 	}

	jmp	SHORT $LN5@ftoa
$LN4@ftoa:

; 88   : 
; 89   : 	*float_ptr = '.';

	mov	rax, QWORD PTR float_ptr$[rsp]
	mov	BYTE PTR [rax], 46			; 0000002eH

; 90   : 	float_ptr++;

	mov	rax, QWORD PTR float_ptr$[rsp]
	inc	rax
	mov	QWORD PTR float_ptr$[rsp], rax

; 91   : 
; 92   : 	float new_value = value - (int)value;

	cvttss2si eax, DWORD PTR value$[rsp]
	cvtsi2ss xmm0, eax
	movss	xmm1, DWORD PTR value$[rsp]
	subss	xmm1, xmm0
	movaps	xmm0, xmm1
	movss	DWORD PTR new_value$[rsp], xmm0

; 93   : 
; 94   : 	for (uint8_t i = 0; i < decimal_places; i++) {

	mov	BYTE PTR i$1[rsp], 0
	jmp	SHORT $LN3@ftoa
$LN2@ftoa:
	movzx	eax, BYTE PTR i$1[rsp]
	inc	al
	mov	BYTE PTR i$1[rsp], al
$LN3@ftoa:
	movzx	eax, BYTE PTR i$1[rsp]
	movzx	ecx, BYTE PTR decimal_places$[rsp]
	cmp	eax, ecx
	jge	SHORT $LN1@ftoa

; 95   : 		new_value *= 10;

	movss	xmm0, DWORD PTR new_value$[rsp]
	mulss	xmm0, DWORD PTR __real@41200000
	movss	DWORD PTR new_value$[rsp], xmm0

; 96   : 		*float_ptr = (int)new_value + 48;

	cvttss2si eax, DWORD PTR new_value$[rsp]
	add	eax, 48					; 00000030H
	mov	rcx, QWORD PTR float_ptr$[rsp]
	mov	BYTE PTR [rcx], al

; 97   : 		new_value -= (int)new_value;

	cvttss2si eax, DWORD PTR new_value$[rsp]
	cvtsi2ss xmm0, eax
	movss	xmm1, DWORD PTR new_value$[rsp]
	subss	xmm1, xmm0
	movaps	xmm0, xmm1
	movss	DWORD PTR new_value$[rsp], xmm0

; 98   : 		float_ptr++;

	mov	rax, QWORD PTR float_ptr$[rsp]
	inc	rax
	mov	QWORD PTR float_ptr$[rsp], rax

; 99   : 	}

	jmp	SHORT $LN2@ftoa
$LN1@ftoa:

; 100  : 
; 101  : 	*float_ptr = 0;

	mov	rax, QWORD PTR float_ptr$[rsp]
	mov	BYTE PTR [rax], 0

; 102  : 
; 103  : 	return float_to_string_output;

	lea	rax, OFFSET FLAT:?float_to_string_output@@3PADA ; float_to_string_output

; 104  : }

	add	rsp, 72					; 00000048H
	ret	0
?ftoa@@YAPEADME@Z ENDP					; ftoa
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\stdio.cpp
_TEXT	SEGMENT
format$ = 48
printf	PROC

; 121  : void printf(const char* format, ...){

$LN3:
	mov	QWORD PTR [rsp+8], rcx
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+24], r8
	mov	QWORD PTR [rsp+32], r9
	sub	rsp, 40					; 00000028H

; 122  : 	AuTextOut(format);

	mov	rcx, QWORD PTR format$[rsp]
	call	AuTextOut

; 123  : }

	add	rsp, 40					; 00000028H
	ret	0
printf	ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\stdio.cpp
_TEXT	SEGMENT
tmp$1 = 0
i$ = 4
z$2 = 8
tv67 = 16
tv74 = 24
value$ = 48
str$ = 56
base$ = 64
?sztoa@@YAPEAD_KPEADH@Z PROC				; sztoa

; 24   : {

$LN11:
	mov	DWORD PTR [rsp+24], r8d
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 25   : 	if (base < 2 || base > 16)

	cmp	DWORD PTR base$[rsp], 2
	jl	SHORT $LN7@sztoa
	cmp	DWORD PTR base$[rsp], 16
	jle	SHORT $LN8@sztoa
$LN7@sztoa:

; 26   : 		return nullptr;

	xor	eax, eax
	jmp	$LN9@sztoa
$LN8@sztoa:

; 27   : 	unsigned int i = 0;

	mov	DWORD PTR i$[rsp], 0
$LN6@sztoa:

; 28   : 	do
; 29   : 	{
; 30   : 		str[++i] = chars[value%base];

	mov	eax, DWORD PTR i$[rsp]
	inc	eax
	mov	DWORD PTR i$[rsp], eax
	mov	eax, DWORD PTR i$[rsp]
	mov	QWORD PTR tv67[rsp], rax
	movsxd	rcx, DWORD PTR base$[rsp]
	xor	edx, edx
	mov	rax, QWORD PTR value$[rsp]
	div	rcx
	mov	rax, rdx
	mov	rcx, QWORD PTR str$[rsp]
	mov	rdx, QWORD PTR chars
	movzx	eax, BYTE PTR [rdx+rax]
	mov	rdx, QWORD PTR tv67[rsp]
	mov	BYTE PTR [rcx+rdx], al

; 31   : 		value /= base;

	movsxd	rax, DWORD PTR base$[rsp]
	mov	QWORD PTR tv74[rsp], rax
	xor	edx, edx
	mov	rax, QWORD PTR value$[rsp]
	mov	rcx, QWORD PTR tv74[rsp]
	div	rcx
	mov	QWORD PTR value$[rsp], rax

; 32   : 	} while (value != 0);

	cmp	QWORD PTR value$[rsp], 0
	jne	SHORT $LN6@sztoa

; 33   : 	str[0] = '\0';

	mov	eax, 1
	imul	rax, rax, 0
	mov	rcx, QWORD PTR str$[rsp]
	mov	BYTE PTR [rcx+rax], 0

; 34   : 	for (unsigned int z = 0; z < i; ++z, --i)

	mov	DWORD PTR z$2[rsp], 0
	jmp	SHORT $LN3@sztoa
$LN2@sztoa:
	mov	eax, DWORD PTR z$2[rsp]
	inc	eax
	mov	DWORD PTR z$2[rsp], eax
	mov	eax, DWORD PTR i$[rsp]
	dec	eax
	mov	DWORD PTR i$[rsp], eax
$LN3@sztoa:
	mov	eax, DWORD PTR i$[rsp]
	cmp	DWORD PTR z$2[rsp], eax
	jae	SHORT $LN1@sztoa

; 35   : 	{
; 36   : 		char tmp = str[z];

	mov	eax, DWORD PTR z$2[rsp]
	mov	rcx, QWORD PTR str$[rsp]
	movzx	eax, BYTE PTR [rcx+rax]
	mov	BYTE PTR tmp$1[rsp], al

; 37   : 		str[z] = str[i];

	mov	eax, DWORD PTR i$[rsp]
	mov	ecx, DWORD PTR z$2[rsp]
	mov	rdx, QWORD PTR str$[rsp]
	mov	r8, QWORD PTR str$[rsp]
	movzx	eax, BYTE PTR [r8+rax]
	mov	BYTE PTR [rdx+rcx], al

; 38   : 		str[i] = tmp;

	mov	eax, DWORD PTR i$[rsp]
	mov	rcx, QWORD PTR str$[rsp]
	movzx	edx, BYTE PTR tmp$1[rsp]
	mov	BYTE PTR [rcx+rax], dl

; 39   : 	}

	jmp	SHORT $LN2@sztoa
$LN1@sztoa:

; 40   : 	return str;

	mov	rax, QWORD PTR str$[rsp]
$LN9@sztoa:

; 41   : }

	add	rsp, 40					; 00000028H
	ret	0
?sztoa@@YAPEAD_KPEADH@Z ENDP				; sztoa
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\stdio.cpp
_TEXT	SEGMENT
n$ = 32
tv70 = 36
neg$ = 40
tv87 = 44
s$ = 64
?atoi@@YAHPEBD@Z PROC					; atoi

; 107  : int atoi(const char* s) {

$LN13:
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 108  : 
; 109  : 	int n = 0, neg = 0;

	mov	DWORD PTR n$[rsp], 0
	mov	DWORD PTR neg$[rsp], 0
$LN8@atoi:

; 110  : 	while (isspace(*s)) s++;

	mov	rax, QWORD PTR s$[rsp]
	movsx	eax, BYTE PTR [rax]
	mov	ecx, eax
	call	isspace
	test	eax, eax
	je	SHORT $LN7@atoi
	mov	rax, QWORD PTR s$[rsp]
	inc	rax
	mov	QWORD PTR s$[rsp], rax
	jmp	SHORT $LN8@atoi
$LN7@atoi:

; 111  : 	switch (*s) {

	mov	rax, QWORD PTR s$[rsp]
	movzx	eax, BYTE PTR [rax]
	mov	BYTE PTR tv70[rsp], al
	cmp	BYTE PTR tv70[rsp], 43			; 0000002bH
	je	SHORT $LN3@atoi
	cmp	BYTE PTR tv70[rsp], 45			; 0000002dH
	je	SHORT $LN4@atoi
	jmp	SHORT $LN5@atoi
$LN4@atoi:

; 112  : 	case '-': neg = 1;

	mov	DWORD PTR neg$[rsp], 1
$LN3@atoi:

; 113  : 	case '+': s++;

	mov	rax, QWORD PTR s$[rsp]
	inc	rax
	mov	QWORD PTR s$[rsp], rax
$LN5@atoi:
$LN2@atoi:

; 114  : 	}
; 115  : 
; 116  : 	while (isdigit(*s))

	mov	rax, QWORD PTR s$[rsp]
	movsx	eax, BYTE PTR [rax]
	mov	ecx, eax
	call	isdigit
	test	eax, eax
	je	SHORT $LN1@atoi

; 117  : 		n = 10 * n - (*s++ - '0');

	imul	eax, DWORD PTR n$[rsp], 10
	mov	rcx, QWORD PTR s$[rsp]
	movsx	ecx, BYTE PTR [rcx]
	sub	ecx, 48					; 00000030H
	sub	eax, ecx
	mov	DWORD PTR n$[rsp], eax
	mov	rax, QWORD PTR s$[rsp]
	inc	rax
	mov	QWORD PTR s$[rsp], rax
	jmp	SHORT $LN2@atoi
$LN1@atoi:

; 118  : 	return neg ? n : -n;

	cmp	DWORD PTR neg$[rsp], 0
	je	SHORT $LN11@atoi
	mov	eax, DWORD PTR n$[rsp]
	mov	DWORD PTR tv87[rsp], eax
	jmp	SHORT $LN12@atoi
$LN11@atoi:
	mov	eax, DWORD PTR n$[rsp]
	neg	eax
	mov	DWORD PTR tv87[rsp], eax
$LN12@atoi:
	mov	eax, DWORD PTR tv87[rsp]

; 119  : }

	add	rsp, 56					; 00000038H
	ret	0
?atoi@@YAHPEBD@Z ENDP					; atoi
_TEXT	ENDS
END
