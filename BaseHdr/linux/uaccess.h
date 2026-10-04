#ifndef __LINUX_UACCESS_H__
#define __LINUX_UACCESS_H__

/*
 * DCL <linux/uaccess.h> -- the user-access family mainline drivers copy in.
 *
 * XenevaOS keeps the calling process's page tables active while it runs a
 * syscall: the devfs bridge hands raw user pointers straight to
 * ->read/->write, and this kernel already dereferences them there. So the
 * copy_*_user family is a direct copy wearing mainline's contract:
 *
 *   copy_to_user/copy_from_user/clear_user  return bytes NOT copied, 0 == ok
 *   access_ok                               returns 1 (no separate range)
 *
 * That keeps a mainline .c honest -- its `if (copy_to_user(...)) return
 * -EFAULT;` branches behave exactly as upstream -- while the memory model
 * stays Xeneva's. A VMA probe can be added behind access_ok later without
 * touching a single caller.
 *
 * Mainline declares these in include/linux/uaccess.h.
 */

#include <stddef.h>
#include <stdint.h>

#ifndef __user
#define __user
#endif

#ifdef __cplusplus
extern "C" {
#endif

unsigned long copy_to_user(void __user* to, const void* from, unsigned long n);
unsigned long copy_from_user(void* to, const void __user* from, unsigned long n);
unsigned long clear_user(void __user* to, unsigned long n);

/* mainline: 0 = success, -EFAULT = the address is not usable */
long copy_from_kernel_nofault(void* dst, const void* src, unsigned long size);

/* no-op in a kernel that never returns into a "failed syscall" state */
static inline void force_successful_syscall_return(void) {}

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_UACCESS_H__ */
