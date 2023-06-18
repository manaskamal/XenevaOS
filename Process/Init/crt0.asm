extern main
extern _KeProcessExit

section .text
[BITS 64]


global __chkstk
__chkstk:
ret

global _aumain
_aumain:
     pop rcx
	 pop rdx
	 sub rsp, 32
	 call main
	 add rsp, 32
	 call _KeProcessExit


	 