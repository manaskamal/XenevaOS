/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2025, Manas Kamal Choudhury
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**/

.global __chkstk 
__chkstk:
    ret

.global _KePrint
_KePrint:
    mov x19,x8
    mov x16, 1
    svc #0
	mov x0, x6
	mov x8,x19
    ret

.global _KePauseThread
_KePauseThread:
    mov x16, 2
    svc #0
	mov x0, x6
    ret

.global _KeGetThreadID
_KeGetThreadID:
    mov x16, 3
    svc #0
	mov x0, x6
    ret


.global _KeGetProcessID
_KeGetProcessID:
    mov x16, 4
    svc #0
	mov x0, x6
    ret

.global _KeProcessExit
_KeProcessExit:
    mov x16, 5
    svc #0
	mov x0, x6
    ret


//===================================
// _KeProcessWaitForTermination: suspends
// currently running process until another
// process state changes
// @param pid -- process id in rcx
//=========================================

.global _KeProcessWaitForTermination
_KeProcessWaitForTermination:
      mov x16, 6
	  svc #0
	  mov x0, x6
	  ret

//=======================================
// _KeCreateProcess -- create a new process
// slot by taking
// @param parent_id -- process parent id
// @param name -- process name
// @ret -- process id of newly create process
// slot
//========================================
.global _KeCreateProcess
_KeCreateProcess:
      mov x16, 7
      svc #0
	  mov x0, x6
	  ret

.global _KeProcessLoadExec
_KeProcessLoadExec:
      mov x16, 8
	  svc #0
	  mov x0, x6
	  ret

.global _KeProcessSleep
_KeProcessSleep:
      mov x16, 23
	  svc #0
	  mov x0, x6
	  ret


//===============================
// _KeSetSignal -- Register a signal
// handler
//===============================
.global _KeSetSignal
_KeSetSignal:
	  mov x16, 25
	  svc #0
	  mov x0, x6
	  ret

.global _KeCreateSharedMem
_KeCreateSharedMem:
      mov x16, 9
	  svc #0
	  mov x0, x6
	  ret

.global _KeObtainSharedMem
_KeObtainSharedMem:
      mov x16, 10
	  svc #0
	  mov x0, x6
	  ret

.global _KeUnmapSharedMem
_KeUnmapSharedMem:
      mov x16, 11
	  svc #0
	  mov x0, x6
	  ret

.global _KeMemMap
_KeMemMap:
	  mov x16, 13
	  svc #0
	  mov x0, x6
	  ret

.global _KeMemUnmap
_KeMemUnmap:
	  mov x16, 14
	  svc #0
	  mov x0, x6
	  ret

//====================================
//_KeGetProcessHeapMem -- Grabs some
// memory from heap area of process
// slot
// @param x0 -- size, number of pages
// multiplied by PAGE_SIZE
//====================================
.global _KeGetProcessHeapMem
_KeGetProcessHeapMem:
	  mov x16, 15
	  svc #0
	  mov x0, x6
	  ret

//=====================================
// _KeOpenFile -- opens a file and
// return a file descriptor
// @param x0 -- file name
// @param x1 -- open mode
//=====================================
.global _KeOpenFile
_KeOpenFile:
	  mov x16, 12
	  svc #0
	  mov x0, x6
	  ret

//====================================
// _KeReadFile -- reads a file into
// buffer
// @param x0 -- file descriptor
// @param x1 -- buffer address
// @param x2 -- length to read in
//====================================
.global _KeReadFile
_KeReadFile:
	  mov x16, 16
	  svc #0
	  mov x0, x6
	  ret

//====================================
// _KeWriteFile -- write a file onto
// buffer
// @param x0 -- file descriptor
// @param x1 -- buffer address
// @param x2 -- length to write out
//====================================
.global _KeWriteFile
_KeWriteFile:
	  mov x16, 17
	  svc #0
	  mov x0, x6
	  ret

//==================================
// _KeCreateDir -- create a new
// directory
// @param x0 -- pathname
//==================================
.global _KeCreateDir
_KeCreateDir:
	  mov x16, 18
	  svc #0
	  mov x0, x6
	  ret


//====================================
// _KeRemoveFile -- remove a file
// from file system
// @param x0 -- filename
//====================================
.global _KeRemoveFile
_KeRemoveFile:
	  mov x16, 19
	  svc #0
	  mov x0, x6
	  ret

//====================================
// _KeCloseFile -- closes a file
// @param x0 -- file descriptor
//====================================
.global _KeCloseFile
_KeCloseFile:
	  mov x16, 20
	  svc #0
	  mov x0, x6
	  ret

.global _KeFileIoControl
_KeFileIoControl:
	  mov x16, 21
	  svc #0
	  mov x0, x6
	  ret

//===============================
// _KeFileStat -- writes the status
// of a file to a buffer
//===============================
.global _KeFileStat
_KeFileStat:
	  mov x16, 22
	  svc #0
	  mov x0, x6
	  ret

//===============================
// _KeGetSystemTimerTick -- get the
// current timer tick value from
// kernel
//===============================
.global _KeGetSystemTimerTick
_KeGetSystemTimerTick:
	  mov x16, 26
	  svc #0
	  mov x0, x6
	  ret

//===============================
// _KeGetFontID -- get a system 
// font id
//===============================
.global _KeGetFontID
_KeGetFontID:
	  mov x16, 27
	  svc #0
	  mov x0, x6
	  ret

//===============================
// _KeGetNumFonts -- get total 
// number of fonts installed
//===============================
.global _KeGetNumFonts
_KeGetNumFonts:
	  mov x16, 28
	  svc #0
	  mov x0, x6
	  ret
      
//===============================
// _KeGetFontSize -- get font size
// by its name
//===============================
.global _KeGetFontSize
_KeGetFontSize:
	  mov x16, 29
	  svc #0
	  mov x0,x6
	  ret


.global _KeMemMapDirty
_KeMemMapDirty:
	  mov x16, 30
	  svc #0
	  mov x0, x6
	  ret

.global _KeCreateTTY
_KeCreateTTY:
	  mov x16, 31
	  svc #0
	  mov x0, x6
	  ret

.global _KeCreateThread
_KeCreateThread:
	  mov x16, 32
	  svc #0
	  mov x0, x6
	  ret

.global _KeSetFileToProcess
_KeSetFileToProcess:
	  mov x16, 33
	  svc #0
	  mov x0, x6
	  ret

.global _KeProcessHeapUnmap
_KeProcessHeapUnmap:
	  mov x16, 34
	  svc #0
	  mov x0, x6
	  ret

.global _KeSendSignal
_KeSendSignal:
	  mov x16, 35
	  svc #0
	  mov x0,x6
	  ret

.global _KeGetCurrentTime
_KeGetCurrentTime:
	 mov x16, 36
	 svc #0
	 mov x0, x6
	 ret

.global _KeOpenDir
_KeOpenDir:
	 mov x16, 37
	 svc #0
	 mov x0, x6
	 ret

.global _KeReadDir
_KeReadDir:
	 mov x16, 38
	 svc #0
	 mov x0,x6
	 ret

.global _KeCreateTimer
_KeCreateTimer:
	 mov x16, 39
	 svc #0
	 mov x0, x6
	 ret

.global _KeStartTimer
_KeStartTimer:
	 mov x16, 40
	 svc #0
	 mov x0,x6
	 ret

.global _KeStopTimer
_KeStopTimer:
	 mov x16, 41
	 svc #0
	 mov x0, x6
	 ret

.global _KeDestroyTimer
_KeDestroyTimer:
	 mov x16, 42
	 svc #0
	 mov x0, x6
	 ret

.global _KeProcessGetFileDesc
_KeProcessGetFileDesc:
	 mov x16, 43
	 svc #0
	 mov x0, x6
	 ret

.global _KeFileSetOffset
_KeFileSetOffset:
	mov x16, 44
	svc #0
	mov x0, x6
	ret


.global gettimeofday
gettimeofday:
	mov x16, 45
	svc #0
	mov x0, x6
	ret

.global socket
socket:
   mov x16, 46
   svc #0
   mov x0, x6
   ret

.global connect
connect:
   mov x16, 47
   svc #0
   mov x0, x6
   ret

.global send
send:
   mov x16, 48
   svc #0
   mov x0, x6
   ret

.global receive
receive:
   mov x16, 49
   svc #0
   mov x0, x6
   ret

.global socket_setopt
socket_setopt:
   mov x16, 50
   // rest will be passed through stack
   svc #0
   mov x0, x6
   ret

.global bind
bind:
   mov x16, 51
   svc #0
   mov x0, x6
   ret

.global accept
accept:
   mov x16, 52
   svc #0
   mov x0, x6
   ret

.global listen
listen:
	mov x16, 53
	svc #0
	mov x0, x6
	ret

.global _KeCreatePipe
_KeCreatePipe:
	mov x16, 54
	svc #0
	mov x0, x6
	ret

.global _KeGetStorageDiskInfo
_KeGetStorageDiskInfo:
	mov x16, 55
	svc #0
	mov x0, x6
	ret

.global _KeGetStoragePartitionInfo
_KeGetStoragePartitionInfo:
	mov x16, 56
	svc #0
	mov x0, x6
	ret

.global _KeGetEnvironmentBlock
_KeGetEnvironmentBlock:
	mov x16, 57
	svc #0
	mov x0, x6
	ret
