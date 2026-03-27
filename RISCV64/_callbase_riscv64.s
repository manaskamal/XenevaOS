/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2023-2026, Manas Kamal Choudhury
 * All rights reserved.
 *
 * _callbase_riscv64.s -- RISC-V 64 system call stubs for XenevaOS
 *
 * Syscall ABI:
 *   a7 = syscall number
 *   a0-a5 = arguments (6 max)
 *   a0 = return value
 *   ecall = trap to kernel
 **/

.text
.align 2

/* __chkstk -- stack probe stub (MSVC compat) */
.global __chkstk
__chkstk:
    ret

/* _KePrint -- syscall 1: debug/kernel print
 * a0 = format string, a1-a5 = args
 */
.global _KePrint
_KePrint:
    li a7, 1
    ecall
    ret

/* _KePauseThread -- syscall 2: block current thread */
.global _KePauseThread
_KePauseThread:
    li a7, 2
    ecall
    ret

/* _KeGetThreadID -- syscall 3: returns current thread id */
.global _KeGetThreadID
_KeGetThreadID:
    li a7, 3
    ecall
    ret

/* _KeGetProcessID -- syscall 4: returns current process id */
.global _KeGetProcessID
_KeGetProcessID:
    li a7, 4
    ecall
    ret

/* _KeProcessExit -- syscall 5: exit current process */
.global _KeProcessExit
_KeProcessExit:
    li a7, 5
    ecall
    ret

/* _KeProcessWaitForTermination -- syscall 6
 * a0 = pid (-1 = any)
 */
.global _KeProcessWaitForTermination
_KeProcessWaitForTermination:
    li a7, 6
    ecall
    ret

/* _KeCreateProcess -- syscall 7
 * a0 = parent_id, a1 = name pointer
 */
.global _KeCreateProcess
_KeCreateProcess:
    li a7, 7
    ecall
    ret

/* _KeProcessLoadExec -- syscall 8
 * a0 = proc_id, a1 = filename, a2 = argc, a3 = argv
 */
.global _KeProcessLoadExec
_KeProcessLoadExec:
    li a7, 8
    ecall
    ret

/* _KeCreateSharedMem -- syscall 9 */
.global _KeCreateSharedMem
_KeCreateSharedMem:
    li a7, 9
    ecall
    ret

/* _KeObtainSharedMem -- syscall 10 */
.global _KeObtainSharedMem
_KeObtainSharedMem:
    li a7, 10
    ecall
    ret

/* _KeUnmapSharedMem -- syscall 11 */
.global _KeUnmapSharedMem
_KeUnmapSharedMem:
    li a7, 11
    ecall
    ret

/* _KeOpenFile -- syscall 12
 * a0 = filename, a1 = mode
 */
.global _KeOpenFile
_KeOpenFile:
    li a7, 12
    ecall
    ret

/* _KeMemMap -- syscall 13 */
.global _KeMemMap
_KeMemMap:
    li a7, 13
    ecall
    ret

/* _KeMemUnmap -- syscall 14 */
.global _KeMemUnmap
_KeMemUnmap:
    li a7, 14
    ecall
    ret

/* _KeGetProcessHeapMem -- syscall 15 */
.global _KeGetProcessHeapMem
_KeGetProcessHeapMem:
    li a7, 15
    ecall
    ret

/* _KeReadFile -- syscall 16
 * a0 = fd, a1 = buffer, a2 = length
 */
.global _KeReadFile
_KeReadFile:
    li a7, 16
    ecall
    ret

/* _KeWriteFile -- syscall 17
 * a0 = fd, a1 = buffer, a2 = length
 */
.global _KeWriteFile
_KeWriteFile:
    li a7, 17
    ecall
    ret

/* _KeCloseFile -- syscall 20 */
.global _KeCloseFile
_KeCloseFile:
    li a7, 20
    ecall
    ret

/* _KeFileIoControl -- syscall 21 */
.global _KeFileIoControl
_KeFileIoControl:
    li a7, 21
    ecall
    ret

/* _KeFileStat -- syscall 22 */
.global _KeFileStat
_KeFileStat:
    li a7, 22
    ecall
    ret

/* _KeProcessSleep -- syscall 23
 * a0 = milliseconds
 */
.global _KeProcessSleep
_KeProcessSleep:
    li a7, 23
    ecall
    ret

/* _KeGetSystemTimerTick -- syscall 26 */
.global _KeGetSystemTimerTick
_KeGetSystemTimerTick:
    li a7, 26
    ecall
    ret

/* _KeGetFontID -- syscall 27 */
.global _KeGetFontID
_KeGetFontID:
    li a7, 27
    ecall
    ret

/* _KeGetNumFonts -- syscall 28 */
.global _KeGetNumFonts
_KeGetNumFonts:
    li a7, 28
    ecall
    ret

/* _KeGetFontSize -- syscall 29 */
.global _KeGetFontSize
_KeGetFontSize:
    li a7, 29
    ecall
    ret

/* _KeMemMapDirty -- syscall 30 */
.global _KeMemMapDirty
_KeMemMapDirty:
    li a7, 30
    ecall
    ret

/* _KeFileSetOffset -- syscall 44
 * a0 = fd, a1 = offset
 */
.global _KeFileSetOffset
_KeFileSetOffset:
    li a7, 44
    ecall
    ret

/* _KeProcessHeapUnmap -- syscall 34 */
.global _KeProcessHeapUnmap
_KeProcessHeapUnmap:
    li a7, 34
    ecall
    ret
