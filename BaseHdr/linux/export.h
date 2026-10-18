#ifndef __LINUX_EXPORT_H__
#define __LINUX_EXPORT_H__

/*
 * DCL <linux/export.h> -- EXPORT_SYMBOL family. DCL compiles ports straight
 * into the kernel image rather than into modules, so exporting is a no-op
 * here; a port built as a .ko resolves symbols through
 * KernelAA64/kernel_exports.c instead. No-ops keep both worlds' source
 * compiling unchanged.
 */

#define EXPORT_SYMBOL(sym)
#define EXPORT_SYMBOL_GPL(sym)
#define EXPORT_SYMBOL_NS(sym, ns)
#define EXPORT_SYMBOL_NS_GPL(sym, ns)

#endif /* __LINUX_EXPORT_H__ */
