; Listing generated by Microsoft (R) Optimizing Compiler Version 18.00.21005.1 

include listing.inc

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

PUBLIC	_fltused
_DATA	SEGMENT
_fltused DD	01H
_DATA	ENDS
PUBLIC	??2@YAPEAX_K@Z					; operator new
PUBLIC	??3@YAXPEAX@Z					; operator delete
PUBLIC	??_U@YAPEAX_K@Z					; operator new[]
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\_crt.cpp
_TEXT	SEGMENT
size$ = 8
??_U@YAPEAX_K@Z PROC					; operator new[]

; 38   : void* __cdecl operator new[](size_t size) {

	mov	QWORD PTR [rsp+8], rcx

; 39   : 	return 0; // malloc(size);

	xor	eax, eax

; 40   : }

	ret	0
??_U@YAPEAX_K@Z ENDP					; operator new[]
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\_crt.cpp
_TEXT	SEGMENT
p$ = 8
??3@YAXPEAX@Z PROC					; operator delete

; 42   : void __cdecl operator delete (void* p) {

	mov	QWORD PTR [rsp+8], rcx

; 43   : 	//free(p);
; 44   : }

	ret	0
??3@YAXPEAX@Z ENDP					; operator delete
_TEXT	ENDS
; Function compile flags: /Odtpy
; File e:\xeneva project\aurora\kernel\_crt.cpp
_TEXT	SEGMENT
size$ = 8
??2@YAPEAX_K@Z PROC					; operator new

; 34   : void* __cdecl ::operator new(size_t size){

	mov	QWORD PTR [rsp+8], rcx

; 35   : 	return 0; // malloc(size);

	xor	eax, eax

; 36   : }

	ret	0
??2@YAPEAX_K@Z ENDP					; operator new
_TEXT	ENDS
END