/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#ifndef __RISCV64_SCHED_H__
#define __RISCV64_SCHED_H__

#include <stdint.h>
#include <aurora.h>
#include <Ipc/signal.h>

#define  THREAD_STATE_READY     1
#define  THREAD_STATE_BLOCKED   3
#define  THREAD_STATE_SLEEP     4
#define  THREAD_STATE_KILLABLE  5

#define  THREAD_LEVEL_KERNEL (1<<0)
#define  THREAD_LEVEL_USER (1<<1)
#define  THREAD_LEVEL_SUBTHREAD (1<<2)
#define  THREAD_LEVEL_MAIN_THREAD (1<<3)

#pragma pack(push,1)
typedef struct _riscv64_frame_ {
    uint64_t ra;
    uint64_t sp;
    uint64_t gp;
    uint64_t tp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t s0;
    uint64_t s1;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
    uint64_t sepc;
    uint64_t sstatus;
    uint64_t stval;
    uint64_t scause;
} AuThreadFrame;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _syscall_param_ {
    int64_t param1;
    int64_t param2;
    int64_t param3;
    int64_t param4;
    int64_t param5;
    int64_t param6;
} AuSyscallParam;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _uentry_ {
    uint64_t entrypoint;
    uint64_t sp;
    int num_args;
    uint64_t argvaddr;
    char** argvs;
    uint64_t stackBase;
} AuUserEntry;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _signal_queue_ {
    void* Signal;
    struct _signal_queue_* link;
} SignalQueue;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _au_thread_ {
    AuThreadFrame frame;
    uint64_t user_stack;
    AuSyscallParam syscall_param;
    uint8_t *fx_state;
    char name[16];
    uint8_t state;
    uint16_t id;
    uint8_t threadType;
    uint8_t first_run;
    uint64_t quanta;
    uint64_t endTick;
    uint8_t priviledge;
    AuSigHandler singals[39];
    SignalQueue* signalQueue;
    uint8_t pendingSigCount;
    void* returnableSignal;
    AuUserEntry *uentry;
    void* procSlot;
    struct _au_thread_ *next;
    struct _au_thread_ *prev;
} AuThread;
#pragma pack(pop)

extern void AuSchedulerStart();
extern void AuSchedulerInitialise();
extern void AuSchedulerInitAp();
AU_EXTERN AU_EXPORT AuThread* AuCreateKthread(void(*entry) (uint64_t), uint64_t stack, uint64_t cr3, char *name);
AU_EXTERN AU_EXPORT AuThread* AuGetCurrentThread();
extern void AuKThreadCopy(AuThread* dest, AuThread* src);
AU_EXTERN AU_EXPORT void AuBlockThread(AuThread *thread);
AU_EXTERN AU_EXPORT void AuSleepThread(AuThread *thread, uint64_t ms);
AU_EXTERN AU_EXPORT void AuUnblockThread(AuThread *t);
extern void AuThreadMoveToTrash(AuThread* t);
extern void AuThreadCleanTrash(AuThread* t);
AU_EXTERN AU_EXPORT AuThread* AuThreadFindByID(uint16_t id);
AU_EXTERN AU_EXPORT AuThread* AuThreadFindByIDBlockList(uint16_t id);
AU_EXTERN AU_EXPORT void AuForceScheduler();
AU_EXTERN AU_EXPORT bool AuIsSchedulerInitialised();
AU_EXTERN AU_EXPORT uint64_t AuGetSystemTimerTick();
AU_EXTERN AU_EXPORT void AuScheduleThread(AuThread* thread);

#endif
