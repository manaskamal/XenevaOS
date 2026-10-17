/*
 * DCL <linux/devres.h> -- managed-device resources without the lifetime.
 *
 * mainline ties every devm_* allocation and action to a struct device's
 * probe-to-remove span and runs them all on device release.  DCL devices
 * have no release: put_device() is a documented no-op (DCL/linux_cdev_shim.c)
 * and no driver in the tree ever unbinds, so the "managed" half reduces to
 * what the call sites actually test.  vkms_drv.c asks devres_open_group()
 * for a non-NULL token before it builds its device and
 * devres_release_group() for a "closed" answer on the way out; drm_drv.c:816
 * returns devm_add_action_or_reset()'s status as the last step of
 * devm_drm_dev_alloc().  Allocations hand back a real allocation -- the
 * caller keeps the pointer on mainline too, and here it is never revoked.
 *
 * This header's include point is <linux/device.h>: mainline spreads these
 * declarations over devres.h and reaches them from device users directly,
 * while DCL's drm chain reaches device.h through drm_print.h, which makes
 * that the one include that covers drm_drv.c, drm_bridge.c and the vkms
 * side (via device/faux.h) without editing a vendored include list.
 */
#ifndef _LINUX_DEVRES_H
#define _LINUX_DEVRES_H

#include <linux/slab.h>	/* kmalloc/kzalloc for the devm_ allocators */

struct device;

static inline void *devres_open_group(struct device *dev, const char *name,
				      gfp_t gfp)
{
	static int dcl_devres_group_marker;

	(void)dev;
	(void)name;
	(void)gfp;
	return &dcl_devres_group_marker;
}

static inline int devres_release_group(struct device *dev, const void *id)
{
	(void)dev;
	(void)id;
	return 1;
}

static inline int devm_add_action_or_reset(struct device *dev,
					   void (*action)(void *),
					   void *data)
{
	(void)dev;
	(void)action;
	(void)data;
	return 0;
}

static inline void devm_release_action(struct device *dev,
				       void (*action)(void *), void *data)
{
	(void)dev;
	(void)action;
	(void)data;
}

static inline void *devm_kmalloc(struct device *dev, size_t size, gfp_t gfp)
{
	(void)dev;
	return kmalloc(size, gfp);
}

static inline void *devm_kzalloc(struct device *dev, size_t size, gfp_t gfp)
{
	(void)dev;
	return kzalloc(size, gfp);
}

/*
 * devres_alloc() / devres_add() / devres_free() -- the three that back
 * devres_open_group()'s siblings above, asked for by drm_drv.c:823-834.
 *
 * The sequence there is the one mainline documents for a private devres
 * entry: allocate a block that carries a release function, either free it
 * again on the failure path or hand it to the device on the success one.
 * `devm_kfree_strarray` is the release; it is never run here, for the reason
 * the header's opening comment gives -- no device in this tree ever releases,
 * so the release half has nowhere to be called from.
 *
 * The block is a real allocation and devres_free() really frees it, because
 * unlike the group tokens above this one is a caller's pointer: drm_drv.c
 * stores it and drm_drv.c drops it, and an "allocation" that leaked or that
 * outlived its free would be a defect rather than a harmless stub. devres_add()
 * is the only empty one of the three, since it exists in mainline to link the
 * block to a device's list and there is no list.
 *
 * dr_release_t is mainline's typedef, kept so a later real devres could be
 * dropped in without the caller's declaration changing.
 */
typedef void (*dr_release_t)(struct device* dev, void* res);

static inline void* devres_alloc(dr_release_t release, size_t size, gfp_t gfp)
{
	(void)release;
	return kmalloc(size, gfp);
}

static inline void devres_add(struct device* dev, void* res)
{
	(void)dev;
	(void)res;
}

static inline void devres_free(void* res)
{
	kfree(res);
}

#endif /* _LINUX_DEVRES_H */
