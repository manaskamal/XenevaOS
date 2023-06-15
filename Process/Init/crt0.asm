extern main
extern exit_proc

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
	 call exit_proc


	 