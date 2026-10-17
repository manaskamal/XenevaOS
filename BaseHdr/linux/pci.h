/*
 * DCL pci.h -- the PCI surface the drm set actually names.
 *
 * There is no PCI on Xeneva (iMX8MP) or on QEMU's `virt` machine, and
 * drm_ioctl.c:152 asks `dev_is_pci(dev->dev)` before every bus-id setup, so
 * the answer is always false and none of the code behind it ever runs. It
 * still has to *compile*, and that is what this header is for: a struct with
 * the three members drm_file.c:991-992 reads, and accessors that answer for
 * a single-bus system that does not exist.
 *
 * The layout carries only what those reads need -- bus, devfn and the struct
 * device mainline's to_pci_dev() subtracts through. It is deliberately not
 * mainline's pci_dev (which is hundreds of bytes and behind an opaque
 * boundary anyway): a partial struct here cannot be mistaken for a real one,
 * because the first thing anyone does with a pci_dev outside this header is
 * pass it to vga_switcheroo_lock_ddc(), which takes it as a pointer and
 * never dereferences it.
 *
 * struct device lives in the pinned <linux/fs.h> (fs.h:184), which is why
 * this header includes fs.h rather than declaring the member opaque: an
 * incomplete struct device cannot be a member.
 */
#ifndef _LINUX_PCI_H
#define _LINUX_PCI_H

#include <linux/types.h>
#include <linux/fs.h>		/* struct device, at fs.h:184 */

/*
 * <linux/capability.h> for capable(), which the three drm sources that name
 * this header also name without including it -- drm_file.c:149 and
 * drm_ioctl.c:152 both write `capable(CAP_SYS_ADMIN)`. In mainline the chain
 * runs through this header's own <linux/device.h> (mainline pci.h:38); DCL's
 * device layer is struct device in the pinned <linux/fs.h> with nothing but a
 * struct body behind it, so the chain stops here and this header names the
 * source directly.
 *
 * Carried here rather than in <linux/file.h>, which is DCL's other carrier of
 * mainline fs.h's vocabulary (and which mainline's fs.h does feed from
 * capability.h), because drm_ioctl.c needs capable() and does not include
 * file.h -- and this header is the one the three consumers all name.
 *
 * Leaf, like every header on this path: capability.h includes only
 * <stdbool.h>.
 */
#include <linux/capability.h>

/*
 * struct pci_bus -- just the bus number. drm_file.c:991 prints
 * `pdev->bus->number` as the middle field of %02x in a BDF string, and
 * pci_domain_nr() below takes the bus, not the device, which is the one thing
 * the previous spelling of this header got wrong: it took a struct pci_dev *
 * and no caller in the tree passes one. drm_file.c:991 passes pdev->bus.
 */
struct pci_bus {
	unsigned char number;
};

struct pci_dev {
	struct pci_bus *bus;
	unsigned int devfn;
	struct device dev;
};

/*
 * PCI_SLOT / PCI_FUNC -- the two bitfields packed into devfn. mainline spells
 * them the same way: slot is bits 7:3, function is bits 2:0. drm_file.c:992
 * prints both, and nothing else in the tree uses them.
 */
#define PCI_SLOT(devfn)		(((devfn) >> 3) & 0xff)
#define PCI_FUNC(devfn)		((devfn) & 0x07)

/*
 * to_pci_dev() -- mainline's spelling, container_of back from the embedded
 * struct device. It is used unguarded at drm_file.c:988 and drm_edid.c:2909
 * and :2938, each immediately *before* an if (!dev_is_pci(...)) early return,
 * so it must produce a usable pointer even though the device is never a PCI
 * device and the block it feeds is never entered.
 */
#define to_pci_dev(n) container_of(n, struct pci_dev, dev)

/*
 * dev_is_pci() -- false, always. mainline's is
 * `((d)->bus == &pci_bus_type)`, and there is no pci_bus_type to compare
 * against because there is no PCI subsystem.
 *
 * A function rather than a macro returning the literal `false`, because
 * drm_file.c:987 and drm_ioctl.c:152 pass `dev->dev`, which may be NULL --
 * drm_ioctl.c:152 writes the NULL check itself (`dev->dev && ...`), but a
 * macro spelling out `false` would leave readers unsure whether the argument
 * is evaluated. This one takes it and ignores it, so the answer does not
 * depend on the pointer.
 */
static inline int dev_is_pci(const struct device *dev)
{
	(void)dev;
	return 0;
}

/*
 * pci_domain_nr() -- 0, which is the correct answer for a single-bus system
 * even if one ever showed up. The argument is a struct pci_bus * (the single
 * caller, drm_file.c:991, passes pdev->bus).
 */
static inline unsigned int pci_domain_nr(const struct pci_bus *bus)
{
	(void)bus;
	return 0;
}

#endif /* _LINUX_PCI_H */
