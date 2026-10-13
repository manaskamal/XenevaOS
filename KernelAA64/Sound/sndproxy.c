/**
 * sndproxy.c -- kernel half of a user-space sound driver.
 *
 * A server process (Process/sndsrv) that owns the sound hardware through
 * /dev/hwdev passes the kernel a shared ring (see BaseHdr/sndsrv.h). The
 * kernel then registers an ordinary sound card, "virtio-sound", whose write()
 * copies the mixer's PCM into that ring instead of touching hardware. The rest
 * of the sound stack (mixer, DSP clients) cannot tell the difference.
 *
 * Safety rules:
 *   - the ring must be one of the caller's own hwdev DMA buffers (AuHwdevOwnsDma)
 *   - write() never spins or waits on the server: it paces itself by sleeping,
 *     and if the server stops draining, data is dropped rather than hanging
 *   - when the server's device handle goes away (exit/crash) the card is
 *     removed and in-flight writers are drained BEFORE its memory is freed
 */
#include <Sound/sndproxy.h>
#include <Sound/sound.h>
#include <Fs/Dev/devhw.h>
#include <Fs/Dev/devfs.h>
#include <Fs/vfs.h>
#include <Cred/group.h>
#include <Mm/kmalloc.h>
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <Drivers/uart.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Hal/AA64/sched.h>
#include <Serv/sysserv.h>
#include <sndsrv.h>
#include <string.h>
#include <_null.h>

extern uint64_t AuGetCurrentMS();

#define PX_DETACH_WAIT_MS   200

static struct {
	AuSound card;
	volatile bool attached;
	volatile int inflight;
	AuProcess* owner;
	SndRingHdr* hdr;
	uint8_t* data;
	uint64_t written, dropped, calls;
	/* audio that was still queued when the server went away; replayed into the
	 * next ring so a crash does not eat what the clients already wrote */
	uint8_t* carry;
	uint32_t carry_len;
	uint64_t carry_ms;           /* when it was taken; stale audio is not replayed */
} px;

#define PX_CARRY_MAX_AGE_MS 1500

static inline void px_barrier(void) {
	__asm__ volatile("dmb ish" ::: "memory");
}

/* The mixer's per-period write: copy into the ring and, once a few periods are
 * queued, put the writer to sleep for about one period of playback time.
 *
 * The consumer is a user process on the same CPU, so this must NEVER spin
 * waiting for it: that starves the very process being waited on (and the
 * compositor). Waiting is done the way ProcessSleep does it in syscall context,
 * by sleeping through the scheduler. A full ring is the last resort: the
 * period is dropped and counted. */
#define PX_BYTES_PER_MS      192      /* 48 kHz * 2 channels * 2 bytes */
#define PX_TARGET_PERIODS    2        /* backlog the writer may keep queued */
#define PX_PACE_MAX_MS       80

static int px_write(uint8_t* buf, size_t len) {
	if (!px.attached || !buf || !len)
		return 1;
	px.inflight++;
	px_barrier();
	int rc = 1;
	uint32_t pace_ms = 0;
	if (px.attached) {
		SndRingHdr* r = px.hdr;
		uint32_t size = r->size;
		if (len > size)
			len = size;
		px.calls++;
		if ((uint32_t)(size - (r->head - r->tail)) < len) {
			r->dropped += (uint32_t)len;
			px.dropped += len;
			pace_ms = PX_PACE_MAX_MS;          /* ring full: we are far ahead of playback */
		} else {
			uint32_t head = r->head;
			uint32_t off = head & (size - 1);
			size_t first = len < (size - off) ? len : (size - off);
			memcpy(px.data + off, buf, first);
			if (first < len)
				memcpy(px.data, buf + first, len - first);
			px_barrier();
			r->head = head + (uint32_t)len;
			px.written += len;
			rc = 0;
			/* sleep in proportion to the backlog above the target, at the playback
			 * rate. Self-correcting: if sleeps come up short, the backlog grows
			 * and the next sleep gets longer. */
			uint32_t fill = r->head - r->tail;
			uint32_t target = PX_TARGET_PERIODS * r->period;
			if (fill > target) {
				pace_ms = (fill - target) / PX_BYTES_PER_MS;
				if (pace_ms < 1)
					pace_ms = 1;
				if (pace_ms > PX_PACE_MAX_MS)
					pace_ms = PX_PACE_MAX_MS;
			}
		}
	}
	px_barrier();
	px.inflight--;
	if (pace_ms) {
		AA64Thread* thr = AuGetCurrentThread();
		if (thr) {
			AuSleepThread(thr, pace_ms);
			AuScheduleThread(AA64GetCurrentRegCtx());
		}
	}
	return rc;
}

static int px_read(uint8_t* buf, size_t len) {
	(void)buf; (void)len;
	return 0;
}

static int px_start(void) {
	if (px.attached)
		px.hdr->running = 1;
	return 0;
}

static int px_stop(void) {
	if (px.attached)
		px.hdr->running = 0;
	return 0;
}

static int px_vol(uint8_t vol) {
	(void)vol;
	return 0;
}

static void px_detach(void) {
	if (!px.attached)
		return;
	px.attached = false;                 /* writers see this and bail out */
	px_barrier();
	AuSoundUnregisterCard(&px.card);
	/* drain writers that were mid-copy before the ring can be freed */
	uint64_t t0 = AuGetCurrentMS();
	while (px.inflight > 0 && AuGetCurrentMS() - t0 < PX_DETACH_WAIT_MS)
		__asm__ volatile("yield");
	if (px.inflight > 0)
		UARTDebugOut("[sndproxy]: %d writer(s) still inside after %d ms\r\n", px.inflight, PX_DETACH_WAIT_MS);
	/* the ring's memory is still valid here (it is freed after this returns):
	 * copy out what the server had not yet consumed */
	if (px.carry) {
		kfree(px.carry);
		px.carry = NULL;
		px.carry_len = 0;
	}
	if (px.hdr && px.data) {
		uint32_t size = px.hdr->size;
		uint32_t pending = px.hdr->head - px.hdr->tail;
		if (pending > size)
			pending = size;
		if (pending) {
			uint8_t* c = (uint8_t*)kmalloc(pending);
			if (c) {
				uint32_t off = px.hdr->tail & (size - 1);
				uint32_t first = pending < (size - off) ? pending : (size - off);
				memcpy(c, px.data + off, first);
				if (first < pending)
					memcpy(c + first, px.data, pending - first);
				px.carry = c;
				px.carry_len = pending;
				px.carry_ms = AuGetCurrentMS();
			}
		}
	}
	px.hdr = NULL;
	px.data = NULL;
	px.owner = NULL;
	UARTDebugOut("[sndproxy]: detached (written %x bytes, dropped %x)\r\n", (uint32_t)px.written, (uint32_t)px.dropped);
}

void AuSndProxyOwnerGone(AuProcess* proc) {
	if (px.attached && px.owner == proc)
		px_detach();
}

static int px_ioctl(AuVFSNode* file, int code, void* arg) {
	(void)file;
	AuProcess* proc = AuHwdevCurrentProcess();
	if (!AuHwdevCallerAllowed(proc))
		return SNDSRV_E_PERM;

	switch (code) {
	case SNDSRV_IOC_ATTACH: {
		uint64_t a = (uint64_t)arg;
		if (!a || a + sizeof(SndsrvAttach) >= 0x0000800000000000ULL)
			return SNDSRV_E_INVAL;
		SndsrvAttach* req = (SndsrvAttach*)arg;
		if (px.attached)
			return SNDSRV_E_BUSY;
		if (req->ring_len < SNDSRV_DATA_OFFSET + 4096 || (req->ring_phys & 0xFFF))
			return SNDSRV_E_INVAL;
		/* never trust a user-supplied physical address: it must be the
		 * caller's own DMA buffer */
		if (!AuHwdevOwnsDma(proc, req->ring_phys, req->ring_len))
			return SNDSRV_E_PERM;
		SndRingHdr* hdr = (SndRingHdr*)P2V(req->ring_phys);
		uint32_t size = hdr->size;
		if (hdr->magic != SNDSRV_RING_MAGIC || size < 4096 || (size & (size - 1)) ||
			(uint64_t)size + SNDSRV_DATA_OFFSET > req->ring_len)
			return SNDSRV_E_INVAL;

		hdr->head = 0;
		hdr->tail = 0;
		hdr->dropped = 0;
		hdr->running = 0;
		/* replay audio left over from a server that died */
		if (px.carry && AuGetCurrentMS() - px.carry_ms > PX_CARRY_MAX_AGE_MS) {
			UARTDebugOut("[sndproxy]: discarded %d bytes of stale carried audio\r\n", (int)px.carry_len);
			kfree(px.carry);
			px.carry = NULL;
			px.carry_len = 0;
		}
		if (px.carry) {
			uint32_t n = px.carry_len < size ? px.carry_len : size;
			memcpy((uint8_t*)hdr + SNDSRV_DATA_OFFSET, px.carry, n);
			hdr->head = n;
			UARTDebugOut("[sndproxy]: carried %d bytes over from the previous server\r\n", (int)n);
			kfree(px.carry);
			px.carry = NULL;
			px.carry_len = 0;
		}
		memset(&px.card, 0, sizeof(px.card));
		strcpy(px.card.name, "virtio-sound");     /* same name the in-kernel driver used */
		px.card.write = px_write;
		px.card.read = px_read;
		px.card.start_output = px_start;
		px.card.stop_output = px_stop;
		px.card.set_vol = px_vol;
		px.card._force_write = 1;
		px.hdr = hdr;
		px.data = (uint8_t*)hdr + SNDSRV_DATA_OFFSET;
		px.owner = proc;
		px.written = px.dropped = px.calls = 0;
		px.inflight = 0;
		px_barrier();
		px.attached = true;
		if (AuSoundRegisterCard(&px.card) != 0) {
			px.attached = false;
			px.hdr = NULL;
			px.data = NULL;
			px.owner = NULL;
			return SNDSRV_E_INVAL;
		}
		UARTDebugOut("[sndproxy]: attached ring %x size %x for pid %d, card registered\r\n",
					 (uint32_t)req->ring_phys, size, proc->proc_id);
		return SNDSRV_OK;
	}
	case SNDSRV_IOC_DETACH:
		if (px.attached && px.owner == proc) {
			px_detach();
			return SNDSRV_OK;
		}
		return SNDSRV_E_INVAL;
	case SNDSRV_IOC_STATS: {
		uint64_t a = (uint64_t)arg;
		if (!a || a + sizeof(SndsrvStats) >= 0x0000800000000000ULL)
			return SNDSRV_E_INVAL;
		SndsrvStats* st = (SndsrvStats*)arg;
		st->written = px.written;
		st->dropped = px.dropped;
		st->calls = px.calls;
		return SNDSRV_OK;
	}
	}
	return SNDSRV_E_INVAL;
}

void AuSndProxyInitialize(void) {
	AuVFSNode* fsys = AuVFSFind("/dev");
	if (!fsys)
		return;
	AuVFSNode* node = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	memset(node, 0, sizeof(AuVFSNode));
	strcpy(node->filename, "sndsrv");
	node->flags = FS_FLAG_DEVICE;
	node->uid = 0;
	node->gid = AuCredGetGroupID(AURORA_GID_MISC_DAEMONS);
	node->device = fsys;
	node->iocontrol = px_ioctl;
	AuDevFSAddFile(fsys, "/", node);
	UARTDebugOut("[sndproxy]: /dev/sndsrv registered\r\n");
}
