/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2023-2026, Manas Kamal Choudhury
* Copyright (c) 2026, Aryan Dadwal
 * All rights reserved.
 *
 * RISC-V 64 System Call Table
 * Mirrors the ARM64 systable.c for functional parity.
 **/

#include <stdint.h>
#include <_null.h>
#include <Serv/sysserv.h>
#include <Mm/shm.h>
#include <Mm/mmap.h>
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <aucon.h>
#include <ftmngr.h>
#include <process.h>
#include <Hal/riscv64_sched.h>

/* Use AURORA_MAX_SYSCALL from sysserv.h */

/* Syscall function format: up to 6 int64 params, returns int64 */
typedef int64_t(*syscall_func)(int64_t, int64_t, int64_t, int64_t, int64_t, int64_t);

static int64_t null_call(int64_t p1, int64_t p2, int64_t p3,
    int64_t p4, int64_t p5, int64_t p6) {
    AuTextOut("Null syscall\r\n");
    return 1;
}

/* SHM Wrappers -- resolve current process then delegate to shm.c */
/* These match the signatures declared in sysserv.h exactly. */

/*
 * CreateSharedMem -- syscall 9
 */
int CreateSharedMem(uint16_t key, size_t sz, uint8_t flags) {
    AuThread* thr = AuGetCurrentThread();
    if (!thr) return -1;
    AuProcess* proc = AuProcessFindThread(thr);
    if (!proc) {
        proc = AuProcessFindSubThread(thr);
        if (!proc) return -1;
    }
    return AuCreateSHM(proc, key, sz, flags);
}

/*
 * ObtainSharedMem -- syscall 10
 */
void* ObtainSharedMem(uint16_t id, void* shmaddr, int shmflg) {
    AuThread* thr = AuGetCurrentThread();
    if (!thr) return 0;
    AuProcess* proc = AuProcessFindThread(thr);
    if (!proc) {
        proc = AuProcessFindSubThread(thr);
        if (!proc) return 0;
    }
    return AuSHMObtainMem(proc, id, shmaddr, shmflg);
}

/*
 * SysUnmapSharedMem -- syscall 11
 */
static int64_t SysUnmapSharedMem(int64_t key, int64_t p1, int64_t p2,
    int64_t p3, int64_t p4, int64_t p5) {
    AuThread* thr = AuGetCurrentThread();
    if (!thr) return -1;
    AuProcess* proc = AuProcessFindThread(thr);
    if (!proc) {
        proc = AuProcessFindSubThread(thr);
        if (!proc) return -1;
    }
    AuSHMUnmap((uint16_t)key, proc);
    return 0;
}

/*
 * GetProcessHeapMem -- syscall 15 (sbrk-like)
 */
uint64_t GetProcessHeapMem(size_t sz) {
    AuThread* thr = AuGetCurrentThread();
    if (!thr) return 0;
    AuProcess* proc = AuProcessFindThread(thr);
    if (!proc) {
        proc = AuProcessFindSubThread(thr);
        if (!proc) return 0;
    }

    if (sz == 0) return 0;
    size_t num_pages = (sz + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t start = proc->proc_mem_heap;
    if (start == 0) {
        start = PROCESS_BREAK_ADDRESS;
        proc->proc_mem_heap = start;
    }

    for (size_t i = 0; i < num_pages; i++) {
        uint64_t phys = (uint64_t)AuPmmngrAlloc();
        AuMapPage(phys, start + i * PAGE_SIZE,
            PTE_READ | PTE_WRITE | PTE_USER);
    }

    uint64_t ret = start;
    proc->proc_mem_heap += num_pages * PAGE_SIZE;
    proc->proc_heapmem_len += num_pages * PAGE_SIZE;
    return ret;
}

/*
 * ProcessHeapUnmap -- syscall 34
 */
int ProcessHeapUnmap(void* ptr, size_t sz) {
    if (!ptr || !sz) return -1;
    sz = (sz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    AuFreePages((uint64_t)ptr, true, sz / PAGE_SIZE);
    return 0;
}

static void* syscalls[AURORA_MAX_SYSCALL] = {
    null_call,                    //0
    AuTextOut,                    //1  -- debug print
    PauseThread,                  //2
    GetThreadID,                  //3
    GetProcessID,                 //4
    ProcessExit,                  //5
    ProcessWaitForTermination,    //6
    CreateProcess,                //7
    ProcessLoadExec,              //8
    CreateSharedMem,              //9
    ObtainSharedMem,              //10
    SysUnmapSharedMem,            //11
    OpenFile,                     //12
    CreateMemMapping,             //13
    UnmapMemMapping,              //14
    GetProcessHeapMem,            //15
    ReadFile,                     //16
    WriteFile,                    //17 -- write to file/device/pipe
    0,                            //18 -- CreateDir (not yet ported)
    0,                            //19 -- RemoveFile (not yet ported)
    CloseFile,                    //20
    FileIoControl,                //21
    FileStat,                     //22
    ProcessSleep,                 //23
    0,                            //24
    0,                            //25
    AuGetSystemTimerTick,         //26
    AuFTMngrGetFontID,            //27
    AuFTMngrGetNumFonts,          //28
    AuFTMngrGetFontSize,          //29
    MemMapDirty,                  //30
    0,                            //31
    0,                            //32
    0,                            //33
    ProcessHeapUnmap,             //34
    0,                            //35
    0,                            //36
    0,                            //37
    0,                            //38
    0,                            //39
    0,                            //40
    0,                            //41
    0,                            //42
    0,                            //43
    FileSetOffset,                //44
    0,                            //45
};

/*
 * AuRISCV64SyscallHandler -- RISC-V system call dispatcher
 * Called from trap handler on EXC_ECALL_FROM_U.
 * @param frame -- pointer to saved trap frame on stack
 *
 * Trap frame layout (from riscv64_lowlevel.s):
 *   offset 64  = a0 (arg0 / return value)
 *   offset 72  = a1 (arg1)
 *   offset 80  = a2 (arg2)
 *   offset 88  = a3 (arg3)
 *   offset 96  = a4 (arg4)
 *   offset 104 = a5 (arg5)
 *   offset 120 = a7 (syscall number)
 *   offset 128 = sepc
 */
void AuRISCV64SyscallHandler(uint64_t* frame) {
    uint64_t syscall_num = frame[15];  /* a7 at offset 120 = index 15 */
    
    if (syscall_num >= AURORA_MAX_SYSCALL) {
        frame[8] = (uint64_t)-1;  /* a0 at offset 64 = index 8 */
        return;
    }

    syscall_func func = (syscall_func)syscalls[syscall_num];
    if (!func) {
        frame[8] = 0;
        return;
    }

    /* Extract arguments from saved registers */
    int64_t arg0 = (int64_t)frame[8];   /* a0 */
    int64_t arg1 = (int64_t)frame[9];   /* a1 */
    int64_t arg2 = (int64_t)frame[10];  /* a2 */
    int64_t arg3 = (int64_t)frame[11];  /* a3 */
    int64_t arg4 = (int64_t)frame[12];  /* a4 */
    int64_t arg5 = (int64_t)frame[13];  /* a5 */

    int64_t retval = func(arg0, arg1, arg2, arg3, arg4, arg5);

    /* Store return value in a0 */
    frame[8] = (uint64_t)retval;
}
