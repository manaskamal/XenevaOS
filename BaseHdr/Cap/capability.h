/**
* @file capability.h
*
* BSD 2-Clause License
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
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

#ifndef __CAPABILITY_H__
#define __CAPABILITY_H__

/**
 * Capability-based security, version 0.
 *
 * This subsystem sits directly on top of the existing file-descriptor
 * architecture rather than replacing it (see Docs design doc
 * "XenevaOS Capability Based Security"). Every process already owns a
 * fixed-size fd table (AuVFSNode* fds[FILE_DESC_PER_PROCESS]) indexed by
 * a plain integer fd. A parallel, same-sized, same-indexed capability
 * table is added to AuProcess (caps[FILE_DESC_PER_PROCESS]) so that:
 *
 *   fds[i]  -> the kernel object the fd refers to (unchanged)
 *   caps[i] -> what THIS process is allowed to do with fds[i]
 *
 * The integer fd keeps working exactly as before for every existing
 * caller; it now additionally acts as the capability's slot id.
 *
 * V0 only targets FS_FLAG_GENERAL / FS_FLAG_DEVICE / FS_FLAG_TTY /
 * FS_FLAG_PIPE objects reachable through AuProcess::fds. Other kernel
 * objects (sockets, shared memory, IPC postboxes, processes) are left
 * for a later version, per the design doc.
 */

#include <stdint.h>
#include <_null.h>
#include <Cred/user.h>

struct _au_proc_;

typedef uint32_t CapRights;

/* -------------------------------------------------------------------- */
/* Rights mask -- section 5 of the design doc.                          */
/* Rights may only ever become MORE restrictive across CapRestrict()/   */
/* inheritance; nothing may add a bit back once it is cleared. (There   */
/* is no CapDup() in V0 -- see the note further down.)                  */
/* -------------------------------------------------------------------- */
#define CAP_READ      (1 << 0)
#define CAP_WRITE     (1 << 1)
/* reserved for executable/process capabilities; not yet consulted
 * anywhere -- V0 has no loader/exec integration */
#define CAP_EXECUTE   (1 << 2)
#define CAP_SEEK      (1 << 3)
#define CAP_IOCTL     (1 << 4)
/* intentionally unreachable in V0: nothing ever sets this bit, and
 * there is no BordoisilaCapDup()/dup() syscall yet either -- an
 * earlier draft had a CapDup() that copied the capability table but
 * left fds[]/caps[] out of sync, so it was removed rather than kept
 * half-working. Reintroduce both together once dup() exists. */
#define CAP_DUP       (1 << 5)
/* reserved for future inter-process capability delegation/transfer;
 * not yet consulted anywhere */
#define CAP_TRANSFER  (1 << 6)

#define CAP_RIGHTS_NONE      0
/* Rights a FILE capability could eventually support once dup/transfer
 * exist -- NOT "rights V0 actually issues" (OpenFile in fileserv.cpp
 * never sets CAP_DUP/CAP_TRANSFER today). This is only an upper bound
 * CapCreate() masks against so a caller can't hand in a garbage mask
 * and get more than the file capability model allows. */
#define CAP_FILE_RIGHTS_MASK  (CAP_READ | CAP_WRITE | CAP_SEEK | CAP_IOCTL | CAP_DUP | CAP_TRANSFER)

/*
 * Return codes for the int-returning capability calls below. Kept
 * local to this subsystem rather than pulled from Libs/XEClib's
 * errno.h -- that header belongs to userland (XEClib), and the rest
 * of the kernel (Kernel/, BaseHdr/) does not reference it, so
 * capability.cpp doesn't either. CAP_ERR_INVALID covers a bad/out of
 * range fd or a slot with no live capability; CAP_ERR_PERM covers a
 * live capability that is missing a rights bit the operation needs.
 */
#define CAP_OK            0
#define CAP_ERR_INVALID  -1
#define CAP_ERR_PERM     -2

/* object types a capability can currently reference */
#define CAP_OBJ_FILE   1

/* capability flags */
#define CAP_FLAG_NONE         0
/* opt out of inheritance on fork/spawn, per section 7 */
#define CAP_FLAG_NO_INHERIT   (1 << 0)

/**
 * AuCapability -- one entry of a process's capability table.
 * Lives at the same index as the fd it governs in AuProcess::fds.
 */
typedef struct _au_capability_ {
	/* reference to the underlying kernel object (an AuVFSNode* for V0) */
	void* object;

	/* CAP_OBJ_* -- what `object` actually is */
	uint8_t object_type;

	/* what THIS capability is allowed to do to `object` */
	CapRights rights;

	/* which uid this capability slot was minted for. Not yet compared
	 * anywhere (CapLookup/CapCheckRights don't check it) -- it exists
	 * so a capability slot records who it was created for, but
	 * whether capability ownership should key off UID, process, or
	 * some future capability-space concept is an open design question
	 * for Manas once transfer/delegation are on the table, not
	 * something this V0 decides. */
	UID_NUM owner;

	/* CAP_FLAG_* */
	uint16_t flags;

	/* set once CapDestroy has run, so a stale lookup can never be
	 * mistaken for a live capability (use-after-close guard) */
	bool valid;
}AuCapability;

/*
 * No refcount field: each fd slot owns one independent AuCapability,
 * not a pointer into something shared. An earlier draft bumped a
 * per-slot refcount on CapDup() and decremented it on CapDestroy(),
 * but a dup'd fd gets its OWN AuCapability at its OWN slot -- bumping
 * the source's counter and then only ever decrementing the copy's
 * left the source's counter permanently stale. If/when capabilities
 * start sharing one underlying object across multiple fds, lifetime
 * for that sharing belongs on the shared object itself (the way
 * AuVFSNode::fileCopyCount already does it for tty), not duplicated
 * here.
 */

/*
 * BordoisilaCapCreate -- mints a new capability for `object` at the
 * given fd slot of `proc`. Called right after a fd-producing operation
 * (open/pipe/tty/dup) decides fds[fd] = object.
 * @param proc -- owning process
 * @param fd -- fd slot to bind the capability to (already allocated
 *              via AuProcessGetFileDesc or reserved 0/1/2)
 * @param object -- underlying kernel object (AuVFSNode* for V0)
 * @param type -- CAP_OBJ_*
 * @param rights -- initial rights mask
 * @return CAP_OK on success, CAP_ERR_INVALID on a bad fd or NULL object
 */
extern int BordoisilaCapCreate(struct _au_proc_* proc, int fd, void* object, uint8_t type, CapRights rights);

/*
 * BordoisilaCapLookup -- returns the capability bound to `fd` in `proc`,
 * or NULL if the slot is empty, out of range, or was destroyed.
 * @param proc -- owning process
 * @param fd -- fd/slot to look up
 */
extern AuCapability* BordoisilaCapLookup(struct _au_proc_* proc, int fd);

/*
 * BordoisilaCapCheckRights -- verifies `fd` in `proc` carries every bit
 * set in `required`. Every capability-aware kernel operation must call
 * this before touching the underlying object.
 * @param proc -- owning process
 * @param fd -- fd/slot to check
 * @param required -- rights mask the operation needs
 * @return true if authorized, false otherwise (missing rights, no
 *         capability bound to fd, or fd owned by a different process)
 */
extern bool BordoisilaCapCheckRights(struct _au_proc_* proc, int fd, CapRights required);

/*
 * NOTE: there is intentionally no BordoisilaCapDup() in V0. An
 * earlier draft duplicated the capability at `caps[fd]` into a new
 * slot but had no way to also duplicate `fds[fd]` -- fd allocation
 * for an *existing* open object isn't something OpenFile or any other
 * current entry point does, so the two tables would go out of sync
 * (caps[new_fd] valid, fds[new_fd] NULL) the instant it was called.
 * caps[]/fds[] must always stay parallel (section 4), so this was
 * removed rather than shipped half-working. Add it back together with
 * a real dup() syscall that duplicates both tables in the same call.
 */

/*
 * BordoisilaCapRestrict -- narrows the rights mask of `fd` in place.
 * Rights can only be cleared here, never set; any bit in `new_rights`
 * that isn't already present in the capability's current rights is
 * silently dropped, so a caller can never use this to escalate.
 * @param proc -- owning process
 * @param fd -- fd/slot to restrict
 * @param new_rights -- desired rights mask (intersected with current)
 * @return CAP_OK on success, CAP_ERR_INVALID if fd has no live capability
 */
extern int BordoisilaCapRestrict(struct _au_proc_* proc, int fd, CapRights new_rights);

/*
 * BordoisilaCapDestroy -- tears down the capability bound to `fd`,
 * immediately marking the slot invalid. Does NOT close/free the
 * underlying kernel object -- that is still the caller's (e.g.
 * CloseFile's) responsibility.
 * @param proc -- owning process
 * @param fd -- fd/slot to destroy
 */
extern void BordoisilaCapDestroy(struct _au_proc_* proc, int fd);

/*
 * BordoisilaCapCleanupProcess -- destroys every live capability owned
 * by `proc`. Called from process exit/kill teardown, alongside the
 * existing fds[] cleanup loop.
 * @param proc -- process being torn down
 */
extern void BordoisilaCapCleanupProcess(struct _au_proc_* proc);

/*
 * BordoisilaCapInheritTable -- copies `parent`'s capability table into
 * `child` right after the child's fds[] have been duplicated (section
 * 7). Rights are copied as-is (never widened); any capability flagged
 * CAP_FLAG_NO_INHERIT is skipped so the child gets an empty slot for
 * that fd instead.
 *
 * NOT CURRENTLY CALLED: AuCreateProcessSlot()/AuCreateProcess() zero
 * fds[] for a new process rather than copying the parent's open
 * files, so there is nothing to inherit yet. This is implemented and
 * ready for when fd inheritance itself lands; wiring it in earlier
 * would silently do nothing and give a false sense of coverage.
 * @param parent -- parent process
 * @param child -- newly created child process
 */
extern void BordoisilaCapInheritTable(struct _au_proc_* parent, struct _au_proc_* child);

#endif