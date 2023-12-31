; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	initialize_list
PUBLIC	list_add
PUBLIC	list_remove
PUBLIC	list_get_at
EXTRN	kmalloc:PROC
EXTRN	kfree:PROC
pdata	SEGMENT
$pdata$initialize_list DD imagerel $LN3
	DD	imagerel $LN3+53
	DD	imagerel $unwind$initialize_list
$pdata$list_add DD imagerel $LN7
	DD	imagerel $LN7+184
	DD	imagerel $unwind$list_add
$pdata$list_remove DD imagerel $LN12
	DD	imagerel $LN12+263
	DD	imagerel $unwind$list_remove
$pdata$list_get_at DD imagerel $LN10
	DD	imagerel $LN10+146
	DD	imagerel $unwind$list_get_at
pdata	ENDS
xdata	SEGMENT
$unwind$initialize_list DD 010401H
	DD	06204H
$unwind$list_add DD 010e01H
	DD	0620eH
$unwind$list_remove DD 010d01H
	DD	0820dH
$unwind$list_get_at DD 010d01H
	DD	0420dH
xdata	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\list.cpp
_TEXT	SEGMENT
current_index$1 = 0
current_node$ = 8
tv75 = 16
list$ = 48
index$ = 56
list_get_at PROC

; 65   : void * list_get_at(list_t* list, unsigned int index) {

$LN10:
	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 40					; 00000028H

; 66   : 
; 67   : 	if (list->pointer == 0 || index >= list->pointer)

	mov	rax, QWORD PTR list$[rsp]
	cmp	DWORD PTR [rax], 0
	je	SHORT $LN4@list_get_a
	mov	rax, QWORD PTR list$[rsp]
	mov	eax, DWORD PTR [rax]
	cmp	DWORD PTR index$[rsp], eax
	jb	SHORT $LN5@list_get_a
$LN4@list_get_a:

; 68   : 		return nullptr;

	xor	eax, eax
	jmp	SHORT $LN6@list_get_a
$LN5@list_get_a:

; 69   : 
; 70   : 	dataentry * current_node = list->entry_current;

	mov	rax, QWORD PTR list$[rsp]
	mov	rax, QWORD PTR [rax+4]
	mov	QWORD PTR current_node$[rsp], rax

; 71   : 
; 72   : 	for (unsigned int current_index = 0; (current_index < index) && current_node; current_index++)

	mov	DWORD PTR current_index$1[rsp], 0
	jmp	SHORT $LN3@list_get_a
$LN2@list_get_a:
	mov	eax, DWORD PTR current_index$1[rsp]
	inc	eax
	mov	DWORD PTR current_index$1[rsp], eax
$LN3@list_get_a:
	mov	eax, DWORD PTR index$[rsp]
	cmp	DWORD PTR current_index$1[rsp], eax
	jae	SHORT $LN1@list_get_a
	cmp	QWORD PTR current_node$[rsp], 0
	je	SHORT $LN1@list_get_a

; 73   : 		current_node = current_node->next;

	mov	rax, QWORD PTR current_node$[rsp]
	mov	rax, QWORD PTR [rax]
	mov	QWORD PTR current_node$[rsp], rax
	jmp	SHORT $LN2@list_get_a
$LN1@list_get_a:

; 74   : 
; 75   : 	return current_node ? current_node->data : nullptr;

	cmp	QWORD PTR current_node$[rsp], 0
	je	SHORT $LN8@list_get_a
	mov	rax, QWORD PTR current_node$[rsp]
	mov	rax, QWORD PTR [rax+16]
	mov	QWORD PTR tv75[rsp], rax
	jmp	SHORT $LN9@list_get_a
$LN8@list_get_a:
	mov	QWORD PTR tv75[rsp], 0
$LN9@list_get_a:
	mov	rax, QWORD PTR tv75[rsp]
$LN6@list_get_a:

; 76   : }

	add	rsp, 40					; 00000028H
	ret	0
list_get_at ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\list.cpp
_TEXT	SEGMENT
current_index$1 = 32
current_node$ = 40
payload$ = 48
list$ = 80
index$ = 88
list_remove PROC

; 78   : void* list_remove(list_t* list, unsigned int index) {

$LN12:
	mov	DWORD PTR [rsp+16], edx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 72					; 00000048H

; 79   : 
; 80   : 	void* payload;
; 81   : 
; 82   : 	if (list->pointer == 0 || index >= list->pointer)

	mov	rax, QWORD PTR list$[rsp]
	cmp	DWORD PTR [rax], 0
	je	SHORT $LN8@list_remov
	mov	rax, QWORD PTR list$[rsp]
	mov	eax, DWORD PTR [rax]
	cmp	DWORD PTR index$[rsp], eax
	jb	SHORT $LN9@list_remov
$LN8@list_remov:

; 83   : 		return nullptr;

	xor	eax, eax
	jmp	$LN10@list_remov
$LN9@list_remov:

; 84   : 
; 85   : 	dataentry* current_node = list->entry_current;

	mov	rax, QWORD PTR list$[rsp]
	mov	rax, QWORD PTR [rax+4]
	mov	QWORD PTR current_node$[rsp], rax

; 86   : 
; 87   : 	for (unsigned int current_index = 0; (current_index < index) && current_node; current_index++)

	mov	DWORD PTR current_index$1[rsp], 0
	jmp	SHORT $LN7@list_remov
$LN6@list_remov:
	mov	eax, DWORD PTR current_index$1[rsp]
	inc	eax
	mov	DWORD PTR current_index$1[rsp], eax
$LN7@list_remov:
	mov	eax, DWORD PTR index$[rsp]
	cmp	DWORD PTR current_index$1[rsp], eax
	jae	SHORT $LN5@list_remov
	cmp	QWORD PTR current_node$[rsp], 0
	je	SHORT $LN5@list_remov

; 88   : 		current_node = current_node->next;

	mov	rax, QWORD PTR current_node$[rsp]
	mov	rax, QWORD PTR [rax]
	mov	QWORD PTR current_node$[rsp], rax
	jmp	SHORT $LN6@list_remov
$LN5@list_remov:

; 89   : 
; 90   : 	if (!current_node)

	cmp	QWORD PTR current_node$[rsp], 0
	jne	SHORT $LN4@list_remov

; 91   : 		return nullptr;

	xor	eax, eax
	jmp	$LN10@list_remov
$LN4@list_remov:

; 92   : 	payload = current_node->data;

	mov	rax, QWORD PTR current_node$[rsp]
	mov	rax, QWORD PTR [rax+16]
	mov	QWORD PTR payload$[rsp], rax

; 93   : 
; 94   : 	if (current_node->prev)

	mov	rax, QWORD PTR current_node$[rsp]
	cmp	QWORD PTR [rax+8], 0
	je	SHORT $LN3@list_remov

; 95   : 		current_node->prev->next = current_node->next;

	mov	rax, QWORD PTR current_node$[rsp]
	mov	rax, QWORD PTR [rax+8]
	mov	rcx, QWORD PTR current_node$[rsp]
	mov	rcx, QWORD PTR [rcx]
	mov	QWORD PTR [rax], rcx
$LN3@list_remov:

; 96   : 
; 97   : 	if (current_node->next)

	mov	rax, QWORD PTR current_node$[rsp]
	cmp	QWORD PTR [rax], 0
	je	SHORT $LN2@list_remov

; 98   : 		current_node->next->prev = current_node->prev;

	mov	rax, QWORD PTR current_node$[rsp]
	mov	rax, QWORD PTR [rax]
	mov	rcx, QWORD PTR current_node$[rsp]
	mov	rcx, QWORD PTR [rcx+8]
	mov	QWORD PTR [rax+8], rcx
$LN2@list_remov:

; 99   : 
; 100  : 	if (index == 0)

	cmp	DWORD PTR index$[rsp], 0
	jne	SHORT $LN1@list_remov

; 101  : 		list->entry_current = current_node->next;

	mov	rax, QWORD PTR list$[rsp]
	mov	rcx, QWORD PTR current_node$[rsp]
	mov	rcx, QWORD PTR [rcx]
	mov	QWORD PTR [rax+4], rcx
$LN1@list_remov:

; 102  : 
; 103  : 	kfree(current_node);

	mov	rcx, QWORD PTR current_node$[rsp]
	call	kfree

; 104  : 
; 105  : 	list->pointer--;

	mov	rax, QWORD PTR list$[rsp]
	mov	eax, DWORD PTR [rax]
	dec	eax
	mov	rcx, QWORD PTR list$[rsp]
	mov	DWORD PTR [rcx], eax

; 106  : 
; 107  : 	return payload;

	mov	rax, QWORD PTR payload$[rsp]
$LN10@list_remov:

; 108  : }

	add	rsp, 72					; 00000048H
	ret	0
list_remove ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\list.cpp
_TEXT	SEGMENT
current_data$ = 32
current_entry$1 = 40
list$ = 64
data$ = 72
list_add PROC

; 42   : void list_add(list_t* list, void* data) {

$LN7:
	mov	QWORD PTR [rsp+16], rdx
	mov	QWORD PTR [rsp+8], rcx
	sub	rsp, 56					; 00000038H

; 43   : 	dataentry *current_data = (dataentry*)kmalloc(sizeof(dataentry));

	mov	ecx, 24
	call	kmalloc
	mov	QWORD PTR current_data$[rsp], rax

; 44   : 	current_data->next = nullptr;

	mov	rax, QWORD PTR current_data$[rsp]
	mov	QWORD PTR [rax], 0

; 45   : 	current_data->prev = nullptr;

	mov	rax, QWORD PTR current_data$[rsp]
	mov	QWORD PTR [rax+8], 0

; 46   : 	current_data->data = data;

	mov	rax, QWORD PTR current_data$[rsp]
	mov	rcx, QWORD PTR data$[rsp]
	mov	QWORD PTR [rax+16], rcx

; 47   : 
; 48   : 
; 49   : 
; 50   : 	if (!list->entry_current) {

	mov	rax, QWORD PTR list$[rsp]
	cmp	QWORD PTR [rax+4], 0
	jne	SHORT $LN4@list_add

; 51   : 		list->entry_current = current_data;

	mov	rax, QWORD PTR list$[rsp]
	mov	rcx, QWORD PTR current_data$[rsp]
	mov	QWORD PTR [rax+4], rcx

; 52   : 	}
; 53   : 	else {

	jmp	SHORT $LN3@list_add
$LN4@list_add:

; 54   : 		dataentry * current_entry = list->entry_current;

	mov	rax, QWORD PTR list$[rsp]
	mov	rax, QWORD PTR [rax+4]
	mov	QWORD PTR current_entry$1[rsp], rax
$LN2@list_add:

; 55   : 		while (current_entry->next) {

	mov	rax, QWORD PTR current_entry$1[rsp]
	cmp	QWORD PTR [rax], 0
	je	SHORT $LN1@list_add

; 56   : 			current_entry = current_entry->next;

	mov	rax, QWORD PTR current_entry$1[rsp]
	mov	rax, QWORD PTR [rax]
	mov	QWORD PTR current_entry$1[rsp], rax

; 57   : 		}

	jmp	SHORT $LN2@list_add
$LN1@list_add:

; 58   : 		current_entry->next = current_data;

	mov	rax, QWORD PTR current_entry$1[rsp]
	mov	rcx, QWORD PTR current_data$[rsp]
	mov	QWORD PTR [rax], rcx

; 59   : 		current_data->prev = current_entry;

	mov	rax, QWORD PTR current_data$[rsp]
	mov	rcx, QWORD PTR current_entry$1[rsp]
	mov	QWORD PTR [rax+8], rcx
$LN3@list_add:

; 60   : 	}
; 61   : 
; 62   : 	list->pointer++;

	mov	rax, QWORD PTR list$[rsp]
	mov	eax, DWORD PTR [rax]
	inc	eax
	mov	rcx, QWORD PTR list$[rsp]
	mov	DWORD PTR [rcx], eax

; 63   : }

	add	rsp, 56					; 00000038H
	ret	0
list_add ENDP
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\list.cpp
_TEXT	SEGMENT
list$ = 32
initialize_list PROC

; 34   : list_t* initialize_list() {

$LN3:
	sub	rsp, 56					; 00000038H

; 35   : 	list_t *list = (list_t*)kmalloc(sizeof(list_t));

	mov	ecx, 12
	call	kmalloc
	mov	QWORD PTR list$[rsp], rax

; 36   : 	list->entry_current = nullptr;

	mov	rax, QWORD PTR list$[rsp]
	mov	QWORD PTR [rax+4], 0

; 37   : 	list->pointer = 0;

	mov	rax, QWORD PTR list$[rsp]
	mov	DWORD PTR [rax], 0

; 38   : 	return list;

	mov	rax, QWORD PTR list$[rsp]

; 39   : }

	add	rsp, 56					; 00000038H
	ret	0
initialize_list ENDP
_TEXT	ENDS
END
