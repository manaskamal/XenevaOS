#ifndef __LINUX_GFP_H__
#define __LINUX_GFP_H__

/*
 * DCL <linux/gfp.h> -- allocation flags.  mainline's is 16.8 KB and brings
 * mmzone.h -> nodemask.h -> numa.h -> percpu behind it, i.e. the entire NUMA
 * surface, in exchange for two constants.
 *
 * DCL already owns both, in <linux/kernel.h>:
 *     :55    typedef unsigned int gfp_t;
 *     :111   #define GFP_KERNEL 0
 *     :112   #define GFP_ATOMIC 1
 * and <linux/slab.h> already accepts mainline's two-argument kmalloc(size,
 * gfp) by dropping the flag (`#define kmalloc(size, flags) dcl_kmalloc(size)`).
 * So there is nothing to declare here -- this header exists so a mainline
 * source that writes `#include <linux/gfp.h>` resolves, and it routes rather
 * than redefines: a second gfp_t and a second GFP_KERNEL in one translation
 * unit would be a redefinition error, and mainline's GFP_KERNEL is a bitmask
 * of __GFP_RECLAIM|__GFP_IO|__GFP_FS that nothing here interprets.
 */
#include <linux/kernel.h>

#endif /* __LINUX_GFP_H__ */
