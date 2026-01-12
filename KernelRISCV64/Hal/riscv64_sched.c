/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#include <Hal/riscv64_sched.h>
#include <Mm/kmalloc.h>
#include <string.h>
#include <aucon.h>
#include <_null.h>

AuThread* _current_thread = NULL;

void AuSchedulerInitialise() {
    // TODO: Initialize scheduler
}

void AuSchedulerStart() {
    // TODO: Start scheduler
}

AU_EXTERN AU_EXPORT AuThread* AuGetCurrentThread() {
    return _current_thread;
}

AU_EXTERN AU_EXPORT AuThread* AuCreateKthread(void(*entry) (uint64_t), uint64_t stack, uint64_t cr3, char *name) {
    AuThread* t = (AuThread*)kmalloc(sizeof(AuThread));
    memset(t, 0, sizeof(AuThread));
    t->frame.sp = stack;
    t->frame.ra = (uint64_t)entry;
    t->id = 1; // TODO: ID alloc
    strcpy(t->name, name);
    return t;
}

AU_EXTERN AU_EXPORT AuThread* AuThreadFindByID(uint16_t id) {
    // TODO: Implement thread list search
    return NULL;
}

AU_EXTERN AU_EXPORT AuThread* AuThreadFindByIDBlockList(uint16_t id) {
    // TODO
    return NULL;
}

AU_EXTERN AU_EXPORT void AuUnblockThread(AuThread *t) {
    t->state = THREAD_STATE_READY;
}

AU_EXTERN AU_EXPORT void AuBlockThread(AuThread *thread) {
    thread->state = THREAD_STATE_BLOCKED;
}

AU_EXTERN AU_EXPORT void AuSleepThread(AuThread *thread, uint64_t ms) {
    // TODO
}

AU_EXTERN AU_EXPORT void AuForceScheduler() {
    // TODO
}

AU_EXTERN AU_EXPORT void AuScheduleThread(AuThread* thread) {
    // TODO
}
