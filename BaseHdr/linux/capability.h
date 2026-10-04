#ifndef __LINUX_CAPABILITY_H__
#define __LINUX_CAPABILITY_H__

/*
 * DCL <linux/capability.h> -- Xeneva does not model Linux capabilities yet.
 * capable() answers yes so mainline driver entry points that gate themselves
 * (open_port() on /dev/mem is the mem.c case) stay reachable; the devfs node
 * permissions remain the real access gate, as they are for every other
 * Xeneva device node.
 */

#define CAP_SYS_RAWIO 17

static inline int capable(int cap) {
	(void)cap;
	return 1;
}

#endif /* __LINUX_CAPABILITY_H__ */
