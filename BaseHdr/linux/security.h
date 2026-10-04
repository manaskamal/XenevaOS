#ifndef __LINUX_SECURITY_H__
#define __LINUX_SECURITY_H__

/*
 * DCL <linux/security.h> -- lockdown hooks only. mainline mem.c gates
 * open_port() on security_locked_down(LOCKDOWN_DEV_MEM); with no LSM in
 * Xeneva the hook always clears. Provided by DCL/linux_mm_shim.c.
 */

#define LOCKDOWN_NONE		0
#define LOCKDOWN_INTEGRITY_MAX	1
#define LOCKDOWN_DEV_MEM	2

int security_locked_down(int reason);

#endif /* __LINUX_SECURITY_H__ */
