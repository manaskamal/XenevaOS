.extern main
.extern _KeProcessExit

.global _aumain
_aumain:
	 sub sp, sp,32
	 bl main
	 add sp, sp,32
	 bl _KeProcessExit