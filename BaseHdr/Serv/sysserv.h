/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
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

#ifndef __SYSSERV_H__
#define __SYSSERV_H__

#include <stdint.h>
#include <stdarg.h>
#include <aurora.h>


/* ==========================================
 *  Threading
 * ==========================================
 */

/*
* PauseThread -- pause the currently running
* thread
*/
extern int PauseThread();

/*
* GetThreadID -- returns currently running thread id
*/
extern uint16_t GetThreadID();

/*
* GetProcessID -- returns currently running process
* id
*/
extern int GetProcessID();

/*
* ProcessExit -- marks a process as died
*/
extern int ProcessExit();

/*
* ProcessWaitForTermination -- wait for termination of
* a child process
* @param pid -- child process id, if -1 then any process
*/
extern int ProcessWaitForTermination(int pid);

/*
* CreateProcess -- creates a new process slot
* @param parent_id -- parent process id
* @param name -- name of the current process slot
*/
extern int CreateProcess(int parent_id, char *name);

/*
* ProcessLoadExec -- loads an executable to a
* process slot
*/
extern int ProcessLoadExec(int proc_id, char* filename, int argc, char** argv);

/*
* CreateSharedMem -- create a shared memory chunk
* @param key -- key to use
* @param sz -- memory size
* @param flags -- shared memory flags
*/
extern int CreateSharedMem(uint16_t key, size_t sz, uint8_t flags);

/*
* ObtainSharedMem -- obtain a shared memory
* @param id -- segment id
* @param shmaddr -- user specified address
* @param shmflg -- flags to use for protection
*/
extern void* ObtainSharedMem(uint16_t id, void* shmaddr, int shmflg);

/*
* UnmapSharedMem -- unmap shared memory segment
* @param key -- key to search
*/
extern void UnmapSharedMem(uint16_t key);

/*
* GetProcessHeapMem -- get a memory from
* process heap
*/
extern uint64_t GetProcessHeapMem(size_t sz);

/*
* OpenFile -- opens a file for user process
* @param file -- file path
* @param mode -- mode of the file
*/
extern int OpenFile(char* filename, int mode);

/*
* ReadFile -- reads a file into given buffer
* @param fd -- file descriptor
* @param buffer -- buffer where to put the data
* @param length -- length in bytes
*/
extern size_t ReadFile(int fd, void* buffer, size_t length);

/*
* WriteFile -- write system call
* @param fd -- file descriptor
* @param buffer -- buffer to write
* @param length -- length in bytes
*/
extern size_t WriteFile(int fd, void* buffer, size_t length);

/*
* CreateDir -- creates a directory
* @param filename -- name of the directory
*/
extern int CreateDir(char* filename);

/*
* RemoveFile -- remove a directory or file
* @param dirname -- directory name
*/
extern int RemoveFile(char* pathname);

#endif