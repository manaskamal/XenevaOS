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
    mov x8, 1
    svc #0
    ret

.global _KePauseThread
_KePauseThread:
    mov x8, 2
    svc #0
    ret

.global _KeGetThreadID
_KeGetThreadID:
    mov x8, 3
    svc #0
    ret


.global _KeGetProcessID
_KeGetProcessID:
    mov x8, 4
    svc #0
    ret

.global _KeProcessExit
_KeProcessExit:
    mov x8, 5
    svc #0
    ret


//===================================
// _KeProcessWaitForTermination: suspends
// currently running process until another
// process state changes
// @param pid -- process id in rcx
//=========================================

.global _KeProcessWaitForTermination
_KeProcessWaitForTermination:
      mov x8, 6
	  svc #0
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
      mov x8, 7
      svc #0
	  ret

.global _KeProcessLoadExec
_KeProcessLoadExec:
      mov x8, 8
	  svc #0
	  ret

.global _KeProcessSleep
_KeProcessSleep:
      mov x8, 23
	  svc #0
	  ret


//===============================
// _KeSetSignal -- Register a signal
// handler
//===============================
.global _KeSetSignal
_KeSetSignal:
	  mov x8, 25
	  svc #0
	  ret

.global _KeCreateSharedMem
_KeCreateSharedMem:
      mov x8, 9
	  svc #0
	  ret

.global _KeObtainSharedMem
_KeObtainSharedMem:
      mov x8, 10
	  svc #0
	  ret

.global _KeUnmapSharedMem
_KeUnmapSharedMem:
      mov x8, 11
	  svc #0
	  ret

.global _KeMemMap
_KeMemMap:
	  mov x8, 13
	  svc #0
	  ret

.global _KeMemUnmap
_KeMemUnmap:
	  mov x8, 14
	  svc #0
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
	  mov x8, 15
	  svc #0
	  ret

//=====================================
// _KeOpenFile -- opens a file and
// return a file descriptor
// @param x0 -- file name
// @param x1 -- open mode
//=====================================
.global _KeOpenFile
_KeOpenFile:
	  mov x8, 12
	  svc #0
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
	  mov x8, 16
	  svc #0
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
	  mov x8, 17
	  svc #0
	  ret

//==================================
// _KeCreateDir -- create a new
// directory
// @param x0 -- pathname
//==================================
.global _KeCreateDir
_KeCreateDir:
	  mov x8, 18
	  svc #0
	  ret


//====================================
// _KeRemoveFile -- remove a file
// from file system
// @param x0 -- filename
//====================================
.global _KeRemoveFile
_KeRemoveFile:
	  mov x8, 19
	  svc #0
	  ret

//====================================
// _KeCloseFile -- closes a file
// @param x0 -- file descriptor
//====================================
.global _KeCloseFile
_KeCloseFile:
	  mov x8, 20
	  svc #0
	  ret

.global _KeFileIoControl
_KeFileIoControl:
	  mov x8, 21
	  svc #0
	  ret

//===============================
// _KeFileStat -- writes the status
// of a file to a buffer
//===============================
.global _KeFileStat
_KeFileStat:
	  mov x8, 22
	  svc #0
	  ret

//===============================
// _KeGetSystemTimerTick -- get the
// current timer tick value from
// kernel
//===============================
.global _KeGetSystemTimerTick
_KeGetSystemTimerTick:
	  mov x8, 26
	  svc #0
	  ret

//===============================
// _KeGetFontID -- get a system 
// font id
//===============================
.global _KeGetFontID
_KeGetFontID:
	  mov x8, 27
	  svc #0
	  ret

//===============================
// _KeGetNumFonts -- get total 
// number of fonts installed
//===============================
.global _KeGetNumFonts
_KeGetNumFonts:
	  mov x8, 28
	  svc #0
	  ret
      
//===============================
// _KeGetFontSize -- get font size
// by its name
//===============================
.global _KeGetFontSize
_KeGetFontSize:
	  mov x8, 29
	  svc #0
	  ret


.global _KeMemMapDirty
_KeMemMapDirty:
	  mov x8, 30
	  svc #0
	  ret

.global _KeCreateTTY
_KeCreateTTY:
	  mov x8, 31
	  svc #0
	  ret

.global _KeCreateThread
_KeCreateThread:
	  mov x8, 32
	  svc #0
	  ret

.global _KeSetFileToProcess
_KeSetFileToProcess:
	  mov x8, 33
	  svc #0
	  ret

.global _KeProcessHeapUnmap
_KeProcessHeapUnmap:
	  mov x8, 34
	  svc #0
	  ret

.global _KeSendSignal
_KeSendSignal:
	  mov x8, 35
	  svc #0
	  ret

.global _KeGetCurrentTime
_KeGetCurrentTime:
	 mov x8, 36
	 svc #0
	 ret

.global _KeOpenDir
_KeOpenDir:
	 mov x8, 37
	 svc #0
	 ret

.global _KeReadDir
_KeReadDir:
	 mov x8, 38
	 svc #0
	 ret

.global _KeCreateTimer
_KeCreateTimer:
	 mov x8, 39
	 svc #0
	 ret

.global _KeStartTimer
_KeStartTimer:
	 mov x8, 40
	 svc #0
	 ret

.global _KeStopTimer
_KeStopTimer:
	 mov x8, 41
	 svc #0
	 ret

.global _KeDestroyTimer
_KeDestroyTimer:
	 mov x8, 42
	 svc #0
	 ret

.global _KeProcessGetFileDesc
_KeProcessGetFileDesc:
	 mov x8, 43
	 svc #0
	 ret

.global _KeFileSetOffset
_KeFileSetOffset:
	mov x8, 44
	svc #0
	ret


.global gettimeofday
gettimeofday:
	mov x8, 45
	svc #0
	ret

.global socket
socket:
   mov x8, 46
   svc #0
   ret

.global connect
connect:
   mov x8, 47
   svc #0
   ret

.global send
send:
   mov x8, 48
   svc #0
   ret

.global receive
receive:
   mov x8, 49
   svc #0
   ret

.global socket_setopt
socket_setopt:
   mov x8, 50
   // rest will be passed through stack
   svc #0
   ret

.global bind
bind:
   mov x8, 51
   svc #0
   ret

.global accept
accept:
   mov x8, 52
   svc #0
   ret

.global listen
listen:
	mov x8, 53
	svc #0
	ret

.global _KeCreatePipe
_KeCreatePipe:
	mov x8, 54
	svc #0
	ret

.global _KeGetStorageDiskInfo
_KeGetStorageDiskInfo:
	mov x8, 55
	svc #0
	ret

.global _KeGetStoragePartitionInfo
_KeGetStoragePartitionInfo:
	mov x8, 56
	svc #0
	ret

.global _KeGetEnvironmentBlock
_KeGetEnvironmentBlock:
	mov x8, 57
	svc #0
	ret
